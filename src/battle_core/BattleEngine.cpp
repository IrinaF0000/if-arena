#include "BattleEngine.hpp"

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
		  _maxTicks(config.maxTicks)
	{
		if (_width <= 0 || _height <= 0)
		{
			throw std::invalid_argument("match dimensions must be positive");
		}
		if (_maxTicks == 0)
		{
			throw std::invalid_argument("maxTicks must be positive");
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
			if (findPlayer(player.player) != nullptr)
			{
				throw std::invalid_argument("duplicate player id");
			}

			_players.push_back(PlayerSnapshot{
				player.player,
				EntityId{nextHeroId},
				player.spawn,
				player.heroHp,
			});
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
			if (pending.command.type != PlayerCommandType::Move)
			{
				continue;
			}

			auto* player = findPlayer(pending.command.player);
			if (player == nullptr)
			{
				continue;
			}

			const Vec2i from = player->position;
			const Vec2i to{
				from.x + pending.command.direction.dx,
				from.y + pending.command.direction.dy,
			};
			if (!inBounds(to) || to == from)
			{
				continue;
			}

			player->position = to;
			events.push_back(BattleEvent{BattleEventType::PlayerMoved, _tick, player->player, from, to});
		}
		_pendingCommands.clear();

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
}
