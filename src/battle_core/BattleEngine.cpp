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
		  _bases(std::move(config.bases))
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
		if (command.type == PlayerCommandType::Attack || command.type == PlayerCommandType::Interact)
		{
			return rejectedResult("command type is not implemented yet");
		}
		if (command.type == PlayerCommandType::Move && !isUnitStep(command.direction))
		{
			return rejectedResult("move direction must be a unit step");
		}

		_pendingCommands.push_back(PendingCommand{command});
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
			const Vec2d target{
				player.worldPosition.x + player.desiredMovement.dx * _playerSpeedPerTick,
				player.worldPosition.y + player.desiredMovement.dy * _playerSpeedPerTick,
			};
			if (!inBounds(target) || collidesWithObstacle(player.worldPosition, target))
			{
				player.inOwnBase = isInOwnBase(player);
				continue;
			}

			player.worldPosition = target;
			player.position = nearestCell(target);
			player.inOwnBase = isInOwnBase(player);
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
		return BattleSnapshot{_tick, _width, _height, _finished, _players};
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
}
