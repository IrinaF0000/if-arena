#include "BattleEngine.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
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

		bool hasDirection(Direction direction)
		{
			return direction.dx != 0 || direction.dy != 0;
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

		double distancePointToSegmentSquared(Vec2d point, Vec2d from, Vec2d to)
		{
			const double segmentX = to.x - from.x;
			const double segmentY = to.y - from.y;
			const double lengthSquared = segmentX * segmentX + segmentY * segmentY;
			if (lengthSquared == 0.0)
			{
				return distanceSquared(from, point);
			}

			const double pointX = point.x - from.x;
			const double pointY = point.y - from.y;
			const double projection = std::clamp((pointX * segmentX + pointY * segmentY) / lengthSquared, 0.0, 1.0);
			const Vec2d closest{from.x + projection * segmentX, from.y + projection * segmentY};
			return distanceSquared(closest, point);
		}

		double distancePointToSegmentSquared(Vec2i point, Vec2d from, Vec2d to)
		{
			return distancePointToSegmentSquared(Vec2d{static_cast<double>(point.x), static_cast<double>(point.y)}, from, to);
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

	PlayerCommand PlayerCommand::attack(PlayerId player, Direction direction)
	{
		return PlayerCommand{player, PlayerCommandType::Attack, direction};
	}

	PlayerCommand PlayerCommand::dash(PlayerId player, Direction direction)
	{
		return PlayerCommand{player, PlayerCommandType::Dash, direction};
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
		  _objectiveConfig(config.objective),
		  _combat(config.combat),
		  _hazardConfigs(std::move(config.hazards))
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
		if (_combat.attackDamage <= 0)
		{
			throw std::invalid_argument("attack damage must be positive");
		}
		if (!std::isfinite(_combat.attackRange) || _combat.attackRange <= 0.0)
		{
			throw std::invalid_argument("attack range must be positive");
		}
		if (!std::isfinite(_combat.dashDistance) || _combat.dashDistance <= 0.0)
		{
			throw std::invalid_argument("dash distance must be positive");
		}
		if (_combat.dashDistance > static_cast<double>(std::max(_width, _height)))
		{
			throw std::invalid_argument("dash distance must fit inside the arena");
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
		if (_hazardConfigs.size() > static_cast<std::size_t>(_width * _height))
		{
			throw std::invalid_argument("hazard count exceeds arena cell count");
		}
		for (const auto& hazard : _hazardConfigs)
		{
			if (!inBounds(hazard.position))
			{
				throw std::invalid_argument("hazard position must be inside match bounds");
			}
			if (!std::isfinite(hazard.radius) || hazard.radius <= 0.0)
			{
				throw std::invalid_argument("hazard radius must be positive");
			}
			if (!std::isfinite(hazard.range) || hazard.range <= 0.0)
			{
				throw std::invalid_argument("hazard range must be positive");
			}
			if (hazard.damage <= 0)
			{
				throw std::invalid_argument("hazard damage must be positive");
			}
			const auto initialPosition =
				hazard.kind == HazardKind::Crow ? crowPatrolPosition(hazard, _hazards.size()) : hazard.position;
			_hazards.push_back(HazardSnapshot{hazard.kind, initialPosition, 0, false});
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
				0,
				0,
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
		const auto* player = findPlayer(command.player);
		if (player->hp <= 0)
		{
			return rejectedResult("player is defeated");
		}
		if (command.type == PlayerCommandType::Attack)
		{
			if (!hasDirection(command.direction) || !isUnitStep(command.direction))
			{
				return rejectedResult("attack direction must be a non-zero unit step");
			}
			if (player->attackCooldownTicksRemaining > 0 || hasPendingCommand(command.player, PlayerCommandType::Attack))
			{
				return rejectedResult("attack is on cooldown");
			}
		}
		if (command.type == PlayerCommandType::Dash)
		{
			if (!hasDirection(command.direction) || !isUnitStep(command.direction))
			{
				return rejectedResult("dash direction must be a non-zero unit step");
			}
			if (player->dashCooldownTicksRemaining > 0 || hasPendingCommand(command.player, PlayerCommandType::Dash))
			{
				return rejectedResult("dash is on cooldown");
			}
		}
		if (command.type == PlayerCommandType::Interact && !canPickupObjective(*player))
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
		updatePlayerCooldowns();
		events.insert(events.end(), _systemEvents.begin(), _systemEvents.end());
		_systemEvents.clear();
		updateObjectiveTimers(events);
		updateObjectiveContact(events);
		for (const auto& pending : _pendingCommands)
		{
			auto* player = findPlayer(pending.command.player);
			if (player == nullptr)
			{
				continue;
			}
			if (player->hp <= 0)
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
			if (pending.command.type == PlayerCommandType::Attack)
			{
				performAttack(*player, pending.command.direction, events);
			}
			if (pending.command.type == PlayerCommandType::Dash)
			{
				performDash(*player, pending.command.direction, events);
			}
			if (pending.command.type == PlayerCommandType::Interact && canPickupObjective(*player))
			{
				pickUpObjective(*player, events);
			}
		}
		_pendingCommands.clear();

		for (auto& player : _players)
		{
			if (player.hp <= 0)
			{
				continue;
			}
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
		updateObjectiveContact(events);
		updateHazards(events);

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
		return BattleSnapshot{_tick, _width, _height, _finished, _players, _obstacles, _objective, _scores, _hazards};
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

	void BattleEngine::updateObjectiveContact(std::vector<BattleEvent>& events)
	{
		if (!_objectiveConfig.has_value())
		{
			return;
		}
		for (auto& player : _players)
		{
			if (player.hp <= 0)
			{
				continue;
			}
			player.inOwnBase = isInOwnBase(player);
			if (canPickupObjective(player))
			{
				pickUpObjective(player, events);
				if (player.inOwnBase)
				{
					captureObjective(player, events);
				}
				return;
			}
		}
	}

	void BattleEngine::dropObjectiveFromSystem(PlayerSnapshot& carrier, std::vector<BattleEvent>& events)
	{
		if (!_objectiveConfig.has_value() || _objective.state != ObjectiveState::Carried ||
		    !(_objective.carrier == carrier.player))
		{
			return;
		}

		_objective.state = ObjectiveState::Dropped;
		_objective.carrier = PlayerId{};
		_objective.position = carrier.worldPosition;
		_objective.pickupLockTicksRemaining = _objectiveConfig->pickupLockTicks;
		_objective.respawnTicksRemaining = 0;
		events.push_back(BattleEvent{BattleEventType::ObjectiveDropped, _tick, carrier.player, carrier.position,
		                             carrier.position, carrier.team, 0, {}, 0});
	}

	void BattleEngine::applyDamage(PlayerSnapshot& target, int damage, std::vector<BattleEvent>& events)
	{
		if (target.hp <= 0)
		{
			return;
		}

		target.hp = std::max(0, target.hp - damage);
		if (_objective.state == ObjectiveState::Carried && _objective.carrier == target.player)
		{
			dropObjectiveFromSystem(target, events);
		}
		if (target.hp == 0)
		{
			target.desiredMovement = MovementVector{};
			events.push_back(BattleEvent{BattleEventType::PlayerDefeated, _tick, target.player, target.position,
			                             target.position, target.team, 0, {}, damage});
		}
	}

	void BattleEngine::performAttack(PlayerSnapshot& attacker, Direction direction, std::vector<BattleEvent>& events)
	{
		attacker.attackCooldownTicksRemaining = _combat.attackCooldownTicks;
		const MovementVector aim = normalize(direction);
		const Vec2d attackEnd{
			attacker.worldPosition.x + aim.dx * _combat.attackRange,
			attacker.worldPosition.y + aim.dy * _combat.attackRange,
		};
		const double hitDistance = 0.5 + _playerCollisionRadius;
		const double hitDistanceSquared = hitDistance * hitDistance;

		PlayerSnapshot* target = nullptr;
		double bestDistanceSquared = _combat.attackRange * _combat.attackRange + 1.0;
		for (auto& candidate : _players)
		{
			if (candidate.player == attacker.player || candidate.team == attacker.team || candidate.hp <= 0)
			{
				continue;
			}
			const double pathDistanceSquared =
				distancePointToSegmentSquared(candidate.worldPosition, attacker.worldPosition, attackEnd);
			const double sourceDistanceSquared = distanceSquared(attacker.worldPosition, candidate.worldPosition);
			if (pathDistanceSquared <= hitDistanceSquared && sourceDistanceSquared <= bestDistanceSquared)
			{
				target = &candidate;
				bestDistanceSquared = sourceDistanceSquared;
			}
		}

		if (target == nullptr)
		{
			events.push_back(BattleEvent{BattleEventType::AttackMissed, _tick, attacker.player, attacker.position,
			                             nearestCell(attackEnd), attacker.team, 0, {}, 0});
			return;
		}

		events.push_back(BattleEvent{BattleEventType::AttackHit, _tick, attacker.player, attacker.position,
		                             target->position, attacker.team, 0, target->player, _combat.attackDamage});
		applyDamage(*target, _combat.attackDamage, events);
	}

	Vec2d BattleEngine::collisionSafeTarget(Vec2d from, Vec2d desired) const
	{
		if (inBounds(desired) && !collidesWithObstacle(from, desired))
		{
			return desired;
		}

		const double distance = std::sqrt(distanceSquared(from, desired));
		const int steps = std::max(1, static_cast<int>(std::ceil(distance * 4.0)));
		Vec2d lastSafe = from;
		for (int step = 1; step <= steps; ++step)
		{
			const double t = static_cast<double>(step) / static_cast<double>(steps);
			const Vec2d candidate{
				from.x + (desired.x - from.x) * t,
				from.y + (desired.y - from.y) * t,
			};
			if (!inBounds(candidate) || collidesWithObstacle(lastSafe, candidate))
			{
				break;
			}
			lastSafe = candidate;
		}
		return lastSafe;
	}

	void BattleEngine::performDash(PlayerSnapshot& player, Direction direction, std::vector<BattleEvent>& events)
	{
		player.dashCooldownTicksRemaining = _combat.dashCooldownTicks;
		const MovementVector dash = normalize(direction);
		const Vec2d target{
			player.worldPosition.x + dash.dx * _combat.dashDistance,
			player.worldPosition.y + dash.dy * _combat.dashDistance,
		};
		const Vec2d safeTarget = collisionSafeTarget(player.worldPosition, target);
		const Vec2i from = player.position;
		player.worldPosition = safeTarget;
		player.position = nearestCell(safeTarget);
		player.inOwnBase = isInOwnBase(player);
		if (_objective.state == ObjectiveState::Carried && _objective.carrier == player.player)
		{
			_objective.position = player.worldPosition;
		}
		if (player.position != from)
		{
			events.push_back(BattleEvent{BattleEventType::PlayerDashed, _tick, player.player, from, player.position,
			                             player.team, 0, {}, 0});
		}
	}

	bool BattleEngine::hasPendingCommand(PlayerId player, PlayerCommandType type) const
	{
		return std::any_of(_pendingCommands.begin(), _pendingCommands.end(), [&](const PendingCommand& pending) {
			return pending.command.player == player && pending.command.type == type;
		});
	}

	void BattleEngine::updatePlayerCooldowns()
	{
		for (auto& player : _players)
		{
			if (player.attackCooldownTicksRemaining > 0)
			{
				--player.attackCooldownTicksRemaining;
			}
			if (player.dashCooldownTicksRemaining > 0)
			{
				--player.dashCooldownTicksRemaining;
			}
		}
	}

	Vec2i BattleEngine::crowPatrolPosition(const HazardConfig& config, std::size_t hazardIndex) const
	{
		constexpr std::array<Vec2i, 6> patrolOffsets{
			Vec2i{1, -1},
			Vec2i{1, 0},
			Vec2i{1, 1},
			Vec2i{-1, 1},
			Vec2i{-1, 0},
			Vec2i{-1, -1},
		};
		const auto phase = static_cast<std::size_t>((config.seed + _tick + static_cast<std::uint32_t>(hazardIndex * 17)) %
		                                            patrolOffsets.size());
		for (std::size_t attempt = 0; attempt < patrolOffsets.size(); ++attempt)
		{
			const auto offset = patrolOffsets[(phase + attempt) % patrolOffsets.size()];
			const Vec2i candidate{config.position.x + offset.x, config.position.y + offset.y};
			const double dx = static_cast<double>(candidate.x - config.position.x);
			const double dy = static_cast<double>(candidate.y - config.position.y);
			if (!inBounds(candidate) || obstacleAt(candidate) || dx * dx + dy * dy > config.range * config.range)
			{
				continue;
			}
			if (_objectiveConfig.has_value() && candidate == _objectiveConfig->spawn)
			{
				continue;
			}
			return candidate;
		}
		return config.position;
	}

	void BattleEngine::updateHazards(std::vector<BattleEvent>& events)
	{
		for (std::size_t index = 0; index < _hazardConfigs.size(); ++index)
		{
			auto& config = _hazardConfigs[index];
			auto& hazard = _hazards[index];
			if (config.kind == HazardKind::Crow)
			{
				hazard.position = crowPatrolPosition(config, index);
			}
			if (hazard.cooldownTicksRemaining > 0)
			{
				--hazard.cooldownTicksRemaining;
				if ((config.kind == HazardKind::Mine || config.kind == HazardKind::Crow) && hazard.cooldownTicksRemaining == 0)
				{
					hazard.triggered = false;
				}
				continue;
			}

			PlayerSnapshot* target = nullptr;
			for (auto& candidate : _players)
			{
				if (candidate.hp <= 0)
				{
					continue;
				}
				const double range = config.kind == HazardKind::Tower ? config.range : config.radius;
				if (distanceSquared(candidate.worldPosition, hazard.position) <= range * range)
				{
					target = &candidate;
					break;
				}
			}
			if (target == nullptr)
			{
				continue;
			}

			hazard.triggered = true;
			hazard.cooldownTicksRemaining = config.cooldownTicks;
			events.push_back(BattleEvent{BattleEventType::HazardTelegraphed, _tick, {}, hazard.position, hazard.position,
			                             target->team, 0, target->player, 0});
			events.push_back(BattleEvent{BattleEventType::HazardHit, _tick, {}, hazard.position, target->position,
			                             target->team, 0, target->player, config.damage});
			applyDamage(*target, config.damage, events);
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
