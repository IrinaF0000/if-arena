// Battle-owned condition handlers.

#include "Features/Battle/Conditions/BattleConditionHandlers.hpp"

#include "Core/Engine/ActionContext.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"

#include <memory>

namespace battle_sim::features::battle
{
	namespace
	{
		class NoAdjacentBlockingCondition final : public core::registry::ICondition
		{
		public:
			bool matches(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				const auto& world = context.world();
				const auto* actorPosition = positionOf(world, actor);
				if (!actorPosition)
				{
					return true;
				}

				for (const auto entity : world.creationOrder())
				{
					if (entity == actor || !world.exists(entity))
					{
						continue;
					}

					const auto* position = positionOf(world, entity);
					if (position && isBlocking(world, entity) && distance(actorPosition->value, position->value) == 1)
					{
						return false;
					}
				}
				return true;
			}
		};

		class HasActionStateCondition final : public core::registry::ICondition
		{
		public:
			bool matches(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule& rule) const override
			{
				const auto* state = context.world().components<ActionStateComponent>().get(actor);
				if (!state || rule.stateKey.empty())
				{
					return false;
				}

				const auto it = state->counters.find(rule.stateKey);
				return it != state->counters.end() && it->second != 0;
			}
		};

		class MissingActionStateCondition final : public core::registry::ICondition
		{
		public:
			bool matches(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule& rule) const override
			{
				const auto* state = context.world().components<ActionStateComponent>().get(actor);
				if (!state || rule.stateKey.empty())
				{
					return true;
				}

				const auto it = state->counters.find(rule.stateKey);
				return it == state->counters.end() || it->second == 0;
			}
		};

		class HasMarchTargetCondition final : public core::registry::ICondition
		{
		public:
			bool matches(core::engine::ActionContext& context, core::ecs::EntityId actor, const core::ecs::ActionRule&) const override
			{
				return context.world().components<MarchTargetComponent>().has(actor);
			}
		};
	}

	void registerBattleConditionHandlers(core::registry::RegistryHub& registries)
	{
		registries.conditions.add("battle.no-adjacent-blocking", std::make_unique<NoAdjacentBlockingCondition>());
		registries.conditions.add("battle.has-action-state", std::make_unique<HasActionStateCondition>());
		registries.conditions.add("battle.missing-action-state", std::make_unique<MissingActionStateCondition>());
		registries.conditions.add("battle.has-march-target", std::make_unique<HasMarchTargetCondition>());
	}
}
