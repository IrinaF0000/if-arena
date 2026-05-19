#include "Features/Battle/Mutations/HealEntityMutation.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"

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

	core::engine::Mutation makeHealEntityMutation(
		core::ecs::EntityId source,
		core::ecs::EntityId target,
		int amount)
	{
		return core::engine::Mutation{
			[source, target, amount](core::engine::GameContext& game) {
				auto& world = game.world;
				require(amount > 0, "Cannot apply non-positive battle heal");
				require(world.exists(source), "Cannot apply battle heal from missing source");
				require(world.exists(target), "Cannot apply battle heal to missing target");

				auto* health = healthOf(world, target);
				require(health != nullptr && health->hp > 0, "Cannot apply battle heal without positive health");

				const auto* sourcePositionComponent = positionOf(world, source);
				const auto* targetPositionComponent = positionOf(world, target);
				const auto sourcePosition = sourcePositionComponent ? sourcePositionComponent->value : core::Position{};
				const auto targetPosition = targetPositionComponent ? targetPositionComponent->value : core::Position{};
				const int previousHealth = health->hp;
				health->hp += amount;
				const int resultingHealth = health->hp;
				game.events.publish(
					features::battle::EffectAppliedEvent{
						game.world.tick(),
						source,
						target,
						features::battle::EffectType::Heal,
						amount,
						resultingHealth,
						sourcePosition,
						targetPosition,
						previousHealth});
			}};
	}
}
