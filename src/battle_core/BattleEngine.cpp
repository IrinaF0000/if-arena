#include "BattleEngine.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace if_arena::battle_core
{
	namespace
	{
		CommandResult acceptedResult()
		{
			return CommandResult{CommandStatus::Accepted, {}};
		}

		CommandResult rejectedResult(std::string reason)
		{
			return CommandResult{CommandStatus::Rejected, std::move(reason)};
		}

		bool isUnitStep(Direction direction)
		{
			return std::abs(direction.dx) <= 1 && std::abs(direction.dy) <= 1;
		}

		bool isZero(MovementVector movement)
		{
			return movement.dx == 0.0 && movement.dy == 0.0;
		}

		MovementVector normalize(Direction direction)
		{
			const double length =
				std::sqrt(static_cast<double>(direction.dx * direction.dx + direction.dy * direction.dy));
			if (length == 0.0)
			{
				return MovementVector{};
			}
			return MovementVector{static_cast<double>(direction.dx) / length, static_cast<double>(direction.dy) / length};
		}

		Vec2i nearestCell(Vec2d position)
		{
			return Vec2i{
				static_cast<int>(std::floor(position.x + 0.5)),
				static_cast<int>(std::floor(position.y + 0.5)),
			};
		}

		double distanceSquared(Vec2d lhs, Vec2i rhs)
		{
			const double dx = lhs.x - static_cast<double>(rhs.x);
			const double dy = lhs.y - static_cast<double>(rhs.y);
			return dx * dx + dy * dy;
		}

		double distanceSquared(Vec2d lhs, Vec2d rhs)
		{
			const double dx = lhs.x - rhs.x;
			const double dy = lhs.y - rhs.y;
			return dx * dx + dy * dy;
		}

		double distancePointToSegmentSquared(Vec2i point, Vec2d from, Vec2d to)
		{
			const double segmentX = to.x - from.x;
			const double segmentY = to.y - from.y;
			const double lengthSquared = segmentX * segmentX + segmentY * segmentY;
			if (lengthSquared == 0.0)
			{
				return distanceSquared(from, point);
			}

			const double pointX = static_cast<double>(point.x) - from.x;
			const double pointY = static_cast<double>(point.y) - from.y;
			const double projection = std::clamp((pointX * segmentX + pointY * segmentY) / lengthSquared, 0.0, 1.0);
			const Vec2d closest{from.x + projection * segmentX, from.y + projection * segmentY};
			return distanceSquared(closest, point);
		}
	}

	PlayerCommand PlayerCommand::move(PlayerId player, Direction direction)
	{
		return PlayerCommand{player, PlayerCommandType::Move, direction};
	}

	PlayerCommand PlayerCommand::stop(PlayerId player)
	{
		return PlayerCommand{player, PlayerCommandType::Stop, {}};
	}

	bool CommandResult::accepted() const
	{
		return status == CommandStatus::Accepted;
	}

	BattleEngine::BattleEngine(MatchConfig config)
		: _width(config.width),
		  _height(config.height),
		  _maxTicks(config.maxTicks),
		  _playerSpeedPerTick(config.playerSpeedPerTick),
		  _playerCollisionRadius(config.playerCollisionRadius),
		  _obstacles(std::move(config.obstacles)),
		  _bases(std::move(config.bases)),
		  _objectiveConfig(config.objective)
	{
		if (_width <= 0 || _height <= 0)
		{
			throw std::invalid_argument("match dimensions must be positive");
		}
		if (_maxTicks == 0)
		{
			throw std::invalid_argument("maxTicks must be positive");
		}
		if (!std::isfinite(_playerSpeedPerTick) || _playerSpeedPerTick <= 0.0)
		{
			throw std::invalid_argument("playerSpeedPerTick must be positive");
		}
		if (!std::isfinite(_playerCollisionRadius) || _playerCollisionRadius < 0.0)
		{
			throw std::invalid_argument("playerCollisionRadius must be non-negative");
		}
		for (const auto& obstacle : _obstacles)
		{
			if (!inBounds(obstacle))
			{
				throw std::invalid_argument("obstacle must be inside match bounds");
			}
		}
		for (const auto& base : _bases)
		{
			if (!inBounds(base.center))
			{
				throw std::invalid_argument("base center must be inside match bounds");
			}
			if (!std::isfinite(base.radius) || base.radius <= 0.0)
			{
				throw std::invalid_argument("base radius must be positive");
			}
		}
		if (_objectiveConfig.has_value())
		{
			if (!inBounds(_objectiveConfig->spawn))
			{
				throw std::invalid_argument("objective spawn must be inside match bounds");
			}
			if (obstacleAt(_objectiveConfig->spawn))
			{
				throw std::invalid_argument("objective spawn must not be inside an obstacle");
			}
			if (!std::isfinite(_objectiveConfig->pickupRadius) || _objectiveConfig->pickupRadius <= 0.0)
			{
				throw std::invalid_argument("objective pickup radius must be positive");
			}
			if (!std::isfinite(_objectiveConfig->carrierSpeedMultiplier) ||
			    _objectiveConfig->carrierSpeedMultiplier <= 0.0 || _objectiveConfig->carrierSpeedMultiplier > 1.0)
			{
				throw std::invalid_argument("objective carrier speed multiplier must be in (0, 1]");
			}
			if (_objectiveConfig->scoreLimit == 0)
			{
				throw std::invalid_argument("objective score limit must be positive");
			}
			_objective = ObjectiveSnapshot{
				ObjectiveState::AtSpawn,
				Vec2d{static_cast<double>(_objectiveConfig->spawn.x), static_cast<double>(_objectiveConfig->spawn.y)},
				PlayerId{},
				0,
				0,
			};
		}

		std::uint32_t nextHeroId = 1;
		for (const auto& player : config.players)
		{
			if (player.player.value == 0)
			{
				throw std::invalid_argument("player id must be non-zero");
			}
			if (player.heroHp <= 0)
			{
				throw std::invalid_argument("hero hp must be positive");
			}
			if (!inBounds(player.spawn))
			{
				throw std::invalid_argument("player spawn must be inside match bounds");
			}
			if (obstacleAt(player.spawn))
			{
				throw std::invalid_argument("player spawn must not be inside an obstacle");
			}
			if (findPlayer(player.player) != nullptr)
			{
				throw std::invalid_argument("duplicate player id");
			}

			PlayerSnapshot snapshot{
				player.player,
				EntityId{nextHeroId},
				player.team,
				player.spawn,
				player.spawn,
				Vec2d{static_cast<double>(player.spawn.x), static_cast<double>(player.spawn.y)},
				MovementVector{},
				player.heroHp,
				false,
			};
			snapshot.inOwnBase = isInOwnBase(snapshot);
			_players.push_back(snapshot);
			if (findScore(player.team) == nullptr)
			{
				_scores.push_back(ScoreSnapshot{player.team, 0});
			}
			++nextHeroId;
		}
	}

	CommandResult BattleEngine::submit(PlayerCommand command)
	{
		if (_finished)
		{
			return rejectedResult("match is finished");
		}
		if (findPlayer(command.player) == nullptr)
		{
			return rejectedResult("unknown player");
		}
		if (command.type == PlayerCommandType::Attack)
		{
			return rejectedResult("command type is not implemented yet");
		}
		if (command.type == PlayerCommandType::Interact && !canPickupObjective(*findPlayer(command.player)))
		{
			return rejectedResult("objective cannot be picked up");
		}
		if (command.type == PlayerCommandType::Move && !isUnitStep(command.direction))
		{
			return rejectedResult("move direction must be a unit step");
		}

		_pendingCommands.push_back(PendingCommand{command});
		return acceptedResult();
	}

	CommandResult BattleEngine::dropObjective(PlayerId carrier)
	{
		if (_finished)
		{
			return rejectedResult("match is finished");
		}
		if (!_objectiveConfig.has_value())
		{
			return rejectedResult("objective is not enabled");
		}
		if (_objective.state != ObjectiveState::Carried || !(_objective.carrier == carrier))
		{
			return rejectedResult("player is not carrying objective");
		}

		const auto* player = findPlayer(carrier);
		if (player == nullptr)
		{
			return rejectedResult("unknown player");
		}

		_objective.state = ObjectiveState::Dropped;
		_objective.carrier = PlayerId{};
		_objective.position = player->worldPosition;
		_objective.pickupLockTicksRemaining = _objectiveConfig->pickupLockTicks;
		_objective.respawnTicksRemaining = 0;
		_systemEvents.push_back(BattleEvent{BattleEventType::ObjectiveDropped, _tick, carrier, player->position,
		                                    player->position, player->team, 0});
		return acceptedResult();
	}

	std::vector<BattleEvent> BattleEngine::tick()
	{
		std::vector<BattleEvent> events;
		if (_finished)
		{
			return events;
		}

		++_tick;
		events.insert(events.end(), _systemEvents.begin(), _systemEvents.end());
		_systemEvents.clear();
		updateObjectiveTimers(events);
		for (const auto& pending : _pendingCommands)
		{
			auto* player = findPlayer(pending.command.player);
			if (player == nullptr)
			{
				continue;
			}

			if (pending.command.type == PlayerCommandType::Stop)
			{
				player->desiredMovement = MovementVector{};
				continue;
			}

			if (pending.command.type == PlayerCommandType::Move)
			{
				player->desiredMovement = normalize(pending.command.direction);
			}
			if (pending.command.type == PlayerCommandType::Interact && canPickupObjective(*player))
			{
				pickUpObjective(*player, events);
			}
		}
		_pendingCommands.clear();

		for (auto& player : _players)
		{
			if (isZero(player.desiredMovement))
			{
				player.inOwnBase = isInOwnBase(player);
				continue;
			}

			const Vec2i from = player.position;
			const double speedMultiplier =
				_objective.state == ObjectiveState::Carried && _objective.carrier == player.player && _objectiveConfig.has_value()
					? _objectiveConfig->carrierSpeedMultiplier
					: 1.0;
			const Vec2d target{
				player.worldPosition.x + player.desiredMovement.dx * _playerSpeedPerTick * speedMultiplier,
				player.worldPosition.y + player.desiredMovement.dy * _playerSpeedPerTick * speedMultiplier,
			};
			if (!inBounds(target) || collidesWithObstacle(player.worldPosition, target))
			{
				player.inOwnBase = isInOwnBase(player);
				continue;
			}

			player.worldPosition = target;
			player.position = nearestCell(target);
			player.inOwnBase = isInOwnBase(player);
			if (_objective.state == ObjectiveState::Carried && _objective.carrier == player.player)
			{
				_objective.position = player.worldPosition;
				if (player.inOwnBase)
				{
					captureObjective(player, events);
				}
			}
			if (player.position != from)
			{
				events.push_back(BattleEvent{BattleEventType::PlayerMoved, _tick, player.player, from, player.position});
			}
		}

		events.push_back(BattleEvent{BattleEventType::TickAdvanced, _tick, {}, {}, {}});
		if (_tick >= _maxTicks)
		{
			_finished = true;
			events.push_back(BattleEvent{BattleEventType::MatchFinished, _tick, {}, {}, {}});
		}

		return events;
	}

	BattleSnapshot BattleEngine::snapshot() const
	{
		return BattleSnapshot{_tick, _width, _height, _finished, _players, _objective, _scores};
	}

	PlayerSnapshot* BattleEngine::findPlayer(PlayerId player)
	{
		for (auto& candidate : _players)
		{
			if (candidate.player == player)
			{
				return &candidate;
			}
		}
		return nullptr;
	}

	const PlayerSnapshot* BattleEngine::findPlayer(PlayerId player) const
	{
		for (const auto& candidate : _players)
		{
			if (candidate.player == player)
			{
				return &candidate;
			}
		}
		return nullptr;
	}

	ScoreSnapshot* BattleEngine::findScore(ArenaTeam team)
	{
		for (auto& score : _scores)
		{
			if (score.team == team)
			{
				return &score;
			}
		}
		return nullptr;
	}

	bool BattleEngine::inBounds(Vec2i position) const
	{
		return position.x >= 0 && position.y >= 0 && position.x < _width && position.y < _height;
	}

	bool BattleEngine::inBounds(Vec2d position) const
	{
		return position.x >= 0.0 && position.y >= 0.0 && position.x <= static_cast<double>(_width - 1) &&
		       position.y <= static_cast<double>(_height - 1);
	}

	bool BattleEngine::obstacleAt(Vec2i cell) const
	{
		return std::find(_obstacles.begin(), _obstacles.end(), cell) != _obstacles.end();
	}

	bool BattleEngine::collidesWithObstacle(Vec2d from, Vec2d to) const
	{
		const double blockedDistance = 0.5 + _playerCollisionRadius;
		const double blockedDistanceSquared = blockedDistance * blockedDistance;
		return std::any_of(_obstacles.begin(), _obstacles.end(), [&](Vec2i obstacle) {
			return distancePointToSegmentSquared(obstacle, from, to) < blockedDistanceSquared;
		});
	}

	bool BattleEngine::isInOwnBase(const PlayerSnapshot& player) const
	{
		const auto base = std::find_if(_bases.begin(), _bases.end(), [&](const BaseZoneConfig& candidate) {
			return candidate.team == player.team;
		});
		if (base == _bases.end())
		{
			return false;
		}
		return distanceSquared(player.worldPosition, base->center) <= base->radius * base->radius;
	}

	bool BattleEngine::canPickupObjective(const PlayerSnapshot& player) const
	{
		if (!_objectiveConfig.has_value())
		{
			return false;
		}
		if (_objective.state != ObjectiveState::AtSpawn && _objective.state != ObjectiveState::Dropped)
		{
			return false;
		}
		if (_objective.pickupLockTicksRemaining > 0)
		{
			return false;
		}

		const double pickupRadiusSquared = _objectiveConfig->pickupRadius * _objectiveConfig->pickupRadius;
		return distanceSquared(player.worldPosition, _objective.position) <= pickupRadiusSquared;
	}

	void BattleEngine::pickUpObjective(PlayerSnapshot& player, std::vector<BattleEvent>& events)
	{
		_objective.state = ObjectiveState::Carried;
		_objective.carrier = player.player;
		_objective.position = player.worldPosition;
		_objective.pickupLockTicksRemaining = 0;
		_objective.respawnTicksRemaining = 0;
		events.push_back(BattleEvent{BattleEventType::ObjectivePickedUp, _tick, player.player, player.position,
		                             player.position, player.team, 0});
	}

	void BattleEngine::captureObjective(PlayerSnapshot& player, std::vector<BattleEvent>& events)
	{
		auto* score = findScore(player.team);
		if (score == nullptr || !_objectiveConfig.has_value())
		{
			return;
		}

		++score->score;
		events.push_back(BattleEvent{BattleEventType::ObjectiveCaptured, _tick, player.player, player.position,
		                             player.position, player.team, score->score});
		events.push_back(BattleEvent{BattleEventType::ScoreChanged, _tick, player.player, player.position, player.position,
		                             player.team, score->score});

		_objective.carrier = PlayerId{};
		_objective.position =
			Vec2d{static_cast<double>(_objectiveConfig->spawn.x), static_cast<double>(_objectiveConfig->spawn.y)};
		if (score->score >= _objectiveConfig->scoreLimit)
		{
			_objective.state = ObjectiveState::Captured;
			_objective.respawnTicksRemaining = 0;
			_finished = true;
			events.push_back(BattleEvent{BattleEventType::MatchFinished, _tick, player.player, player.position,
			                             player.position, player.team, score->score});
			return;
		}

		_objective.state = ObjectiveState::Respawning;
		_objective.respawnTicksRemaining = _objectiveConfig->captureRespawnDelayTicks;
		if (_objective.respawnTicksRemaining == 0)
		{
			_objective.state = ObjectiveState::AtSpawn;
			events.push_back(BattleEvent{BattleEventType::ObjectiveRespawned, _tick, {}, _objectiveConfig->spawn,
			                             _objectiveConfig->spawn, {}, 0});
		}
	}

	void BattleEngine::updateObjectiveTimers(std::vector<BattleEvent>& events)
	{
		if (!_objectiveConfig.has_value())
		{
			return;
		}
		if (_objective.pickupLockTicksRemaining > 0)
		{
			--_objective.pickupLockTicksRemaining;
		}
		if (_objective.state == ObjectiveState::Respawning && _objective.respawnTicksRemaining > 0)
		{
			--_objective.respawnTicksRemaining;
			if (_objective.respawnTicksRemaining == 0)
			{
				_objective.state = ObjectiveState::AtSpawn;
				_objective.position =
					Vec2d{static_cast<double>(_objectiveConfig->spawn.x), static_cast<double>(_objectiveConfig->spawn.y)};
				events.push_back(BattleEvent{BattleEventType::ObjectiveRespawned, _tick, {}, _objectiveConfig->spawn,
				                             _objectiveConfig->spawn, {}, 0});
			}
		}
	}
}
