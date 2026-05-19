#include "Features/Battle/Mutations/RemoveBattleEntityMutation.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
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

	core::engine::Mutation makeRemoveBattleEntityMutation(
		core::ecs::EntityId entity,
		features::battle::RemovalReason reason,
		bool publishEvent)
	{
		return core::engine::Mutation{
			[entity, reason, publishEvent](core::engine::GameContext& game) {
				auto& world = game.world;
				require(world.exists(entity), "Cannot remove missing battle entity");

				const auto* removedPositionComponent = positionOf(world, entity);
				const auto removedPosition = removedPositionComponent ? removedPositionComponent->value : core::Position{};
				if (game.resources.contains<BattleMapResource>() && isBlocking(world, entity))
				{
					if (const auto* position = positionOf(world, entity))
					{
						game.resources.get<BattleMapResource>().remove(entity, position->value);
					}
				}

				world.components().alive.remove(entity);
				require(world.removeEntity(entity), "Cannot remove battle entity from world");

				if (publishEvent)
				{
					game.events.publish(features::battle::EntityRemovedEvent{game.world.tick(), entity, reason, removedPosition});
				}
			}};
	}
}
