#pragma once

#include "CoordinateTransform.hpp"

#include <QString>
#include <QVector>

#include <cstdint>

namespace if_arena::battle_qt_client::game
{
	struct PlayerSnapshot
	{
		QString playerId;
		QString heroId;
		Team team{Team::Unknown};
		double x{};
		double y{};
		int hp{};
		int attackCooldown{};
		int dashCooldown{};
		bool inOwnBase{};
	};

	struct ObjectiveSnapshot
	{
		QString state{"unknown"};
		double x{};
		double y{};
		QString carrierPlayerId;
		int pickupLockTicks{};
		int respawnTicks{};
	};

	struct ScoreSnapshot
	{
		Team team{Team::Unknown};
		int score{};
	};

	struct HazardSnapshot
	{
		QString id;
		QString kind;
		QString visualId;
		double x{};
		double y{};
		double radius{};
		double range{};
		int damage{};
		QString effect;
		QString trigger;
		QString icon;
		bool blocksMovement{};
		bool causesDrop{};
		double rangeRadius{};
		QString team;
		int cooldownTicks{};
		int cooldown{};
		bool triggered{};
	};

	struct ObstacleSnapshot
	{
		QString id;
		QString kind;
		QString visualId;
		double x{};
		double y{};
		bool blocksMovement{};
		int damage{};
		bool causesDrop{};
		double rangeRadius{};
		int cooldownTicks{};
		int cooldown{};
		QString team;
	};

	struct ScenarioMetadata
	{
		QString id;
		QString mode;
		int version{};
		QString source;
	};

	struct ArenaSnapshot
	{
		QString matchId;
		std::uint64_t tick{};
		bool finished{};
		ScenarioMetadata scenario;
		MapSize map{21.0, 13.0};
		QVector<ObstacleSnapshot> obstacles;
		QVector<PlayerSnapshot> players;
		ObjectiveSnapshot objective;
		QVector<ScoreSnapshot> scores;
		QVector<HazardSnapshot> hazards;
	};

	struct MatchJoined
	{
		QString matchId;
		QString matchCode;
		ScenarioMetadata scenario;
	};

	enum class ClientIntentKind
	{
		Move,
		Aim,
		Attack,
		Dash,
		Interact,
		Stop
	};

	struct ClientIntent
	{
		ClientIntentKind kind{ClientIntentKind::Stop};
		Direction direction{};
	};

	[[nodiscard]] Team teamFromString(const QString& value);
	[[nodiscard]] QString teamName(Team team);
	[[nodiscard]] QString intentKindName(ClientIntentKind kind);
}
