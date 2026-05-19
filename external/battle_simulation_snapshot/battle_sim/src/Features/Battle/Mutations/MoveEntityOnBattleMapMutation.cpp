#include "Features/Battle/Mutations/MoveEntityOnBattleMapMutation.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <stdexcept>

namespace battle_sim::features::battle
{
	namespace
	{
		void require(bool condition, const char* message)
		{
			if (!condition)
			{
				throw std::runtime_error(message);
			}
		}
	}

	core::engine::Mutation makeMoveEntityOnBattleMapMutation(
		core::ecs::EntityId entity,
		core::Position from,
		core::Position to,
		bool occupiesCell,
		bool reachesMarchTarget)
	{
		return core::engine::Mutation{
			[entity, from, to, occupiesCell, reachesMarchTarget](core::engine::GameContext& game) {
				auto& world = game.world;
				require(world.exists(entity), "Cannot move missing battle entity");

				auto* position = positionOf(world, entity);
				require(position != nullptr, "Cannot move battle entity without position");
				require(position->value == from, "Cannot move battle entity from stale position");

				auto& map = game.resources.get<BattleMapResource>();
				require(map.inside(to), "Cannot move battle entity outside map");
				if (occupiesCell)
				{
					require(map.entityAt(from).value_or(0) == entity, "Cannot move battle entity from stale map cell");
					map.move(entity, from, to);
				}

				position->value = to;
				if (reachesMarchTarget)
				{
					world.components<MarchTargetComponent>().remove(entity);
				}
			},
			[entity, from, to, reachesMarchTarget](core::engine::GameContext& game) {
				game.events.publish(features::battle::EntityMovedEvent{game.world.tick(), entity, from, to});
				if (reachesMarchTarget)
				{
					game.events.publish(features::battle::MarchEndedEvent{game.world.tick(), entity, to});
				}
			}};
	}
}
