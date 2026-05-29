#include "ProtocolJson.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>

namespace if_arena::battle_qt_client::game
{
	namespace
	{
		template <typename T>
		ParseResult<T> fail(QString message)
		{
			return ParseResult<T>{std::nullopt, ParseFailure{std::move(message)}};
		}

		std::optional<QJsonObject> parseObject(const QString& payloadJson, QString& error)
		{
			QJsonParseError parseError{};
			const auto document = QJsonDocument::fromJson(payloadJson.toUtf8(), &parseError);
			if (parseError.error != QJsonParseError::NoError || !document.isObject())
			{
				error = "invalid JSON object";
				return std::nullopt;
			}
			return document.object();
		}

		bool requireString(const QJsonObject& object, const QString& key, QString& value)
		{
			const auto json = object.value(key);
			if (!json.isString())
			{
				return false;
			}
			value = json.toString();
			return !value.isEmpty();
		}

		bool requireNumber(const QJsonObject& object, const QString& key, double& value)
		{
			const auto json = object.value(key);
			if (!json.isDouble())
			{
				return false;
			}
			value = json.toDouble();
			return true;
		}

		bool requireInt(const QJsonObject& object, const QString& key, int& value)
		{
			double number{};
			if (!requireNumber(object, key, number))
			{
				return false;
			}
			value = static_cast<int>(number);
			return true;
		}

		bool requireBool(const QJsonObject& object, const QString& key, bool& value)
		{
			const auto json = object.value(key);
			if (!json.isBool())
			{
				return false;
			}
			value = json.toBool();
			return true;
		}

		bool parseScenarioMetadata(const QJsonObject& object, ScenarioMetadata& metadata)
		{
			const auto scenario = object.value("scenario");
			if (!scenario.isObject())
			{
				return false;
			}
			const auto scenarioObject = scenario.toObject();
			return requireString(scenarioObject, "id", metadata.id) &&
			       requireString(scenarioObject, "mode", metadata.mode) &&
			       requireInt(scenarioObject, "version", metadata.version) &&
			       requireString(scenarioObject, "source", metadata.source);
		}
	}

	ParseResult<ArenaSnapshot> parseArenaSnapshotPayload(const QString& payloadJson)
	{
		QString error;
		const auto root = parseObject(payloadJson, error);
		if (!root.has_value())
		{
			return fail<ArenaSnapshot>(error);
		}

		ArenaSnapshot snapshot;
		if (!requireString(*root, "matchId", snapshot.matchId))
		{
			return fail<ArenaSnapshot>("snapshot missing matchId");
		}
		double tick{};
		if (!requireNumber(*root, "tick", tick))
		{
			return fail<ArenaSnapshot>("snapshot missing tick");
		}
		snapshot.tick = static_cast<std::uint64_t>(tick);
		bool finished{};
		if (requireBool(*root, "finished", finished))
		{
			snapshot.finished = finished;
		}
		if (!parseScenarioMetadata(*root, snapshot.scenario))
		{
			return fail<ArenaSnapshot>("snapshot scenario metadata is invalid");
		}

		const auto map = root->value("map");
		if (!map.isObject())
		{
			return fail<ArenaSnapshot>("snapshot missing map");
		}
		const auto mapObject = map.toObject();
		if (!requireNumber(mapObject, "width", snapshot.map.width) ||
		    !requireNumber(mapObject, "height", snapshot.map.height) || snapshot.map.width <= 0.0 ||
		    snapshot.map.height <= 0.0)
		{
			return fail<ArenaSnapshot>("snapshot map is invalid");
		}

		const auto obstacles = root->value("obstacles");
		if (!obstacles.isArray())
		{
			return fail<ArenaSnapshot>("snapshot missing obstacles");
		}
		for (const auto& obstacleValue : obstacles.toArray())
		{
			if (!obstacleValue.isObject())
			{
				return fail<ArenaSnapshot>("snapshot obstacle is invalid");
			}
			const auto obstacleObject = obstacleValue.toObject();
			ObstacleSnapshot obstacle;
			if (!requireString(obstacleObject, "id", obstacle.id) ||
			    !requireString(obstacleObject, "kind", obstacle.kind) ||
			    !requireString(obstacleObject, "visualId", obstacle.visualId) ||
			    !requireNumber(obstacleObject, "x", obstacle.x) || !requireNumber(obstacleObject, "y", obstacle.y) ||
			    !requireBool(obstacleObject, "blocksMovement", obstacle.blocksMovement) ||
			    !requireInt(obstacleObject, "damage", obstacle.damage) ||
			    !requireBool(obstacleObject, "causesDrop", obstacle.causesDrop) ||
			    !requireNumber(obstacleObject, "rangeRadius", obstacle.rangeRadius) ||
			    !requireInt(obstacleObject, "cooldownTicks", obstacle.cooldownTicks) ||
			    !requireInt(obstacleObject, "cooldown", obstacle.cooldown) ||
			    !requireString(obstacleObject, "team", obstacle.team))
			{
				return fail<ArenaSnapshot>("snapshot obstacle fields are invalid");
			}
			if (obstacle.kind != "blocking_obstacle" || obstacle.visualId.isEmpty() || !obstacle.blocksMovement ||
			    obstacle.damage != 0 || obstacle.causesDrop || obstacle.rangeRadius != 0.0 ||
			    obstacle.cooldownTicks != 0 || obstacle.cooldown != 0 || obstacle.team != "neutral")
			{
				return fail<ArenaSnapshot>("snapshot obstacle metadata is invalid");
			}
			snapshot.obstacles.push_back(obstacle);
		}

		const auto players = root->value("players");
		if (!players.isArray())
		{
			return fail<ArenaSnapshot>("snapshot missing players");
		}
		for (const auto& playerValue : players.toArray())
		{
			if (!playerValue.isObject())
			{
				return fail<ArenaSnapshot>("snapshot player is invalid");
			}
			const auto playerObject = playerValue.toObject();
			PlayerSnapshot player;
			QString team;
			if (!requireString(playerObject, "playerId", player.playerId) ||
			    !requireString(playerObject, "heroId", player.heroId) || !requireString(playerObject, "team", team) ||
			    !requireNumber(playerObject, "x", player.x) || !requireNumber(playerObject, "y", player.y) ||
			    !requireInt(playerObject, "hp", player.hp) ||
			    !requireInt(playerObject, "attackCooldown", player.attackCooldown) ||
			    !requireInt(playerObject, "dashCooldown", player.dashCooldown) ||
			    !requireBool(playerObject, "inOwnBase", player.inOwnBase))
			{
				return fail<ArenaSnapshot>("snapshot player fields are invalid");
			}
			player.team = teamFromString(team);
			if (player.team == Team::Unknown)
			{
				return fail<ArenaSnapshot>("snapshot player team is invalid");
			}
			snapshot.players.push_back(player);
		}

		const auto objective = root->value("objective");
		if (!objective.isObject())
		{
			return fail<ArenaSnapshot>("snapshot missing objective");
		}
		const auto objectiveObject = objective.toObject();
		if (!requireString(objectiveObject, "state", snapshot.objective.state) ||
		    !requireNumber(objectiveObject, "x", snapshot.objective.x) ||
		    !requireNumber(objectiveObject, "y", snapshot.objective.y) ||
		    !requireString(objectiveObject, "carrierPlayerId", snapshot.objective.carrierPlayerId) ||
		    !requireInt(objectiveObject, "pickupLockTicks", snapshot.objective.pickupLockTicks) ||
		    !requireInt(objectiveObject, "respawnTicks", snapshot.objective.respawnTicks))
		{
			return fail<ArenaSnapshot>("snapshot objective fields are invalid");
		}

		const auto scores = root->value("scores");
		if (scores.isArray())
		{
			for (const auto& scoreValue : scores.toArray())
			{
				if (!scoreValue.isObject())
				{
					return fail<ArenaSnapshot>("snapshot score is invalid");
				}
				const auto scoreObject = scoreValue.toObject();
				QString team;
				ScoreSnapshot score;
				if (!requireString(scoreObject, "team", team) || !requireInt(scoreObject, "score", score.score))
				{
					return fail<ArenaSnapshot>("snapshot score fields are invalid");
				}
				score.team = teamFromString(team);
				snapshot.scores.push_back(score);
			}
		}

		const auto hazards = root->value("hazards");
		if (hazards.isArray())
		{
			for (const auto& hazardValue : hazards.toArray())
			{
				if (!hazardValue.isObject())
				{
					return fail<ArenaSnapshot>("snapshot hazard is invalid");
				}
				const auto hazardObject = hazardValue.toObject();
				HazardSnapshot hazard;
				if (!requireString(hazardObject, "id", hazard.id) || !requireString(hazardObject, "kind", hazard.kind) ||
				    !requireString(hazardObject, "visualId", hazard.visualId) ||
				    !requireNumber(hazardObject, "x", hazard.x) || !requireNumber(hazardObject, "y", hazard.y) ||
				    !requireNumber(hazardObject, "radius", hazard.radius) ||
				    !requireNumber(hazardObject, "range", hazard.range) ||
				    !requireInt(hazardObject, "damage", hazard.damage) ||
				    !requireString(hazardObject, "effect", hazard.effect) ||
				    !requireString(hazardObject, "trigger", hazard.trigger) ||
				    !requireString(hazardObject, "icon", hazard.icon) ||
				    !requireBool(hazardObject, "blocksMovement", hazard.blocksMovement) ||
				    !requireBool(hazardObject, "causesDrop", hazard.causesDrop) ||
				    !requireNumber(hazardObject, "rangeRadius", hazard.rangeRadius) ||
				    !requireString(hazardObject, "team", hazard.team) ||
				    !requireInt(hazardObject, "cooldownTicks", hazard.cooldownTicks) ||
				    !requireInt(hazardObject, "cooldown", hazard.cooldown) ||
				    !requireBool(hazardObject, "triggered", hazard.triggered))
				{
					return fail<ArenaSnapshot>("snapshot hazard fields are invalid");
				}
				if ((hazard.kind != "mine" && hazard.kind != "tower" && hazard.kind != "crow") ||
				    (hazard.effect != "damage" && hazard.effect != "damage_drop_objective") ||
				    (hazard.trigger != "proximity" && hazard.trigger != "range") || hazard.icon.isEmpty() ||
				    hazard.visualId.isEmpty() || hazard.blocksMovement || hazard.team != "neutral")
				{
					return fail<ArenaSnapshot>("snapshot hazard metadata is invalid");
				}
				const auto expectedRangeRadius = hazard.trigger == "range" ? hazard.range : hazard.radius;
				const auto expectedCausesDrop = hazard.effect == "damage_drop_objective";
				if (hazard.rangeRadius != expectedRangeRadius || hazard.causesDrop != expectedCausesDrop)
				{
					return fail<ArenaSnapshot>("snapshot hazard metadata is invalid");
				}
				snapshot.hazards.push_back(hazard);
			}
		}

		return ParseResult<ArenaSnapshot>{snapshot, std::nullopt};
	}

	ParseResult<MatchJoined> parseMatchJoinedPayload(const QString& payloadJson)
	{
		QString error;
		const auto root = parseObject(payloadJson, error);
		if (!root.has_value())
		{
			return fail<MatchJoined>(error);
		}
		MatchJoined joined;
		if (!requireString(*root, "matchId", joined.matchId) || !requireString(*root, "matchCode", joined.matchCode) ||
		    !parseScenarioMetadata(*root, joined.scenario))
		{
			return fail<MatchJoined>("match_joined payload is invalid");
		}
		return ParseResult<MatchJoined>{joined, std::nullopt};
	}

	ParseResult<QString> parseSessionIdPayload(const QString& payloadJson)
	{
		QString error;
		const auto root = parseObject(payloadJson, error);
		if (!root.has_value())
		{
			return fail<QString>(error);
		}
		bool accepted{};
		QString sessionId;
		if (!requireBool(*root, "accepted", accepted) || !accepted || !requireString(*root, "sessionId", sessionId))
		{
			return fail<QString>("auth_result payload is invalid");
		}
		return ParseResult<QString>{sessionId, std::nullopt};
	}

	ParseResult<QString> parseErrorMessagePayload(const QString& payloadJson)
	{
		QString error;
		const auto root = parseObject(payloadJson, error);
		if (!root.has_value())
		{
			return fail<QString>(error);
		}
		QString code;
		QString message;
		if (!requireString(*root, "code", code) || !requireString(*root, "message", message))
		{
			return fail<QString>("server error payload is invalid");
		}
		return ParseResult<QString>{code + ": " + message, std::nullopt};
	}

	QJsonObject intentPayload(const QString& matchId, const ClientIntent& intent)
	{
		QJsonObject command;
		command.insert("kind", intentKindName(intent.kind));
		if (intent.kind == ClientIntentKind::Move || intent.kind == ClientIntentKind::Aim ||
		    intent.kind == ClientIntentKind::Attack || intent.kind == ClientIntentKind::Dash)
		{
			QJsonObject direction;
			const auto serverDirection = localDirectionToServerCommand(intent.direction);
			direction.insert("x", serverDirection.dx);
			direction.insert("y", serverDirection.dy);
			command.insert("direction", direction);
		}

		QJsonObject payload;
		payload.insert("matchId", matchId);
		payload.insert("command", command);
		return payload;
	}
}
