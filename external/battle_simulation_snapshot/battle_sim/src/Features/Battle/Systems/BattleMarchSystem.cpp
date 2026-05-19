// Battle-owned march command system.

#include "Features/Battle/Systems/BattleMarchSystem.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <iostream>

namespace battle_sim::features::battle
{
	bool BattleMarchSystem::start(core::EntityId entity, core::Position target, core::engine::GameContext& game, core::Tick tick)
	{
		if (entity == 0)
		{
			std::cerr << "[stderr] march: entityId == 0\n";
			return false;
		}
		if (!game.resources.contains<BattleMapResource>())
		{
			std::cerr << "[stderr] march: map not created\n";
			return false;
		}

		const auto& map = game.resources.get<BattleMapResource>();
		if (!map.inside(target))
		{
			std::cerr << "[stderr] march: target out of map\n";
			return false;
		}
		if (!game.world.exists(entity))
		{
			std::cerr << "[stderr] march: entity not found\n";
			return false;
		}
		if (!game.world.components<MobileComponent>().has(entity) || !isAlive(game.world, entity))
		{
			std::cerr << "[stderr] march: entity cannot move\n";
			return false;
		}

		const auto* position = positionOf(game.world, entity);
		if (!position)
		{
			return false;
		}

		game.events.publish(features::battle::MarchStartedEvent{tick, entity, position->value, target});
		if (target == position->value)
		{
			game.events.publish(features::battle::MarchEndedEvent{tick, entity, position->value});
			game.world.components<MarchTargetComponent>().remove(entity);
		}
		else
		{
			game.world.components<MarchTargetComponent>().emplace(entity, MarchTargetComponent{target});
		}
		return true;
	}
}
