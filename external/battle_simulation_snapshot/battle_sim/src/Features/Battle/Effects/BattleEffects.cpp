// Battle-owned effect handlers.

#include "Features/Battle/Effects/BattleEffects.hpp"

#include "Core/Engine/ActionContext.hpp"
#include "Core/Engine/Mutation.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Mutations/DamageEntityMutation.hpp"
#include "Features/Battle/Mutations/HealEntityMutation.hpp"
#include "Features/Battle/Mutations/RemoveBattleEntityMutation.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"
#include "Features/Battle/Systems/BattleMovementSystem.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

namespace battle_sim::features::battle
{
	namespace
	{
		bool physicalRadiusRule(const core::ecs::ActionRule& rule)
		{
			return rule.selectorHandler.has_value() && rule.selectorHandler->str() == "battle.physical-radius";
		}

		int stepToward(int from, int to, int step)
		{
			if (from == to || step <= 0)
			{
				return from;
			}
			if (from < to)
			{
				return std::min(from + step, to);
			}
			return std::max(from - step, to);
		}

		class DamageEffect final : public core::registry::IEffect
		{
		public:
			bool apply(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				core::ecs::EntityId target,
				const core::ecs::ActionRule& rule,
				int amount) const override
			{
				auto& world = context.world();
				if (amount <= 0 || !world.exists(actor) || !world.exists(target))
				{
					return false;
				}

				auto* health = healthOf(world, target);
				if (!health || health->hp <= 0)
				{
					return false;
				}

				if (!physicalRadiusRule(rule))
				{
					if (!isAttackable(world, target))
					{
						return false;
					}
				}

				try
				{
					core::engine::MutationQueue::applyNow(context.game(), makeDamageEntityMutation(actor, target, amount));
				}
				catch (const std::exception&)
				{
					return false;
				}
				return true;
			}
		};

		class HealEffect final : public core::registry::IEffect
		{
		public:
			bool apply(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				core::ecs::EntityId target,
				const core::ecs::ActionRule&,
				int amount) const override
			{
				auto& world = context.world();
				if (amount <= 0 || !world.exists(actor) || !world.exists(target))
				{
					return false;
				}

				auto* health = healthOf(world, target);
				if (!health || health->hp <= 0)
				{
					return false;
				}

				try
				{
					core::engine::MutationQueue::applyNow(context.game(), makeHealEntityMutation(actor, target, amount));
				}
				catch (const std::exception&)
				{
					return false;
				}
				return true;
			}
		};

		class MoveEffect final : public core::registry::IEffect
		{
		public:
			bool apply(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				core::ecs::EntityId,
				const core::ecs::ActionRule& rule,
				int amount) const override
			{
				auto& world = context.world();
				if (!world.exists(actor))
				{
					return false;
				}

				auto* position = positionOf(world, actor);
				const auto* target = world.components<MarchTargetComponent>().get(actor);
				if (!position || !target)
				{
					return false;
				}

				const auto from = position->value;
				const int step = amount > 0 ? amount : rule.stepDistance;
				if (context.game().resources.contains<BattleMapResource>())
				{
					return BattleMovementSystem::apply(context, actor, step);
				}

				const auto to = core::Position{
					stepToward(from.x, target->target.x, step),
					stepToward(from.y, target->target.y, step)};
				if (to == from)
				{
					return false;
				}

				try
				{
					core::engine::MutationQueue::applyNow(
						context.game(),
						core::engine::Mutation{
							[actor, from, to](core::engine::GameContext& game) {
								auto* currentPosition = positionOf(game.world, actor);
								if (!game.world.exists(actor) || !currentPosition || currentPosition->value != from)
								{
									throw std::runtime_error("Cannot move battle entity from stale position");
								}
								currentPosition->value = to;
							},
							[actor, from, to](core::engine::GameContext& game) {
								game.events.publish(features::battle::EntityMovedEvent{game.world.tick(), actor, from, to});
							}});
				}
				catch (const std::exception&)
				{
					return false;
				}
				return true;
			}
		};

		class SetActionStateEffect final : public core::registry::IEffect
		{
		public:
			bool apply(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				core::ecs::EntityId,
				const core::ecs::ActionRule& rule,
				int) const override
			{
				if (rule.stateKey.empty())
				{
					return false;
				}
				auto& actionState = context.world().components<ActionStateComponent>();
				if (!actionState.has(actor))
				{
					actionState.emplace(actor, features::battle::ActionStateComponent{});
				}
				actionState.get(actor)->counters.insert_or_assign(rule.stateKey, rule.stateValue);
				return true;
			}
		};

		class DestroySelfEffect final : public core::registry::IEffect
		{
		public:
			bool apply(
				core::engine::ActionContext& context,
				core::ecs::EntityId actor,
				core::ecs::EntityId,
				const core::ecs::ActionRule&,
				int) const override
			{
				auto& world = context.world();
				if (!world.exists(actor))
				{
					return false;
				}

				try
				{
					core::engine::MutationQueue::applyNow(
						context.game(),
						makeRemoveBattleEntityMutation(actor, features::battle::RemovalReason::Destroyed, true));
				}
				catch (const std::exception&)
				{
					return false;
				}
				return true;
			}
		};
	}

	void registerBattleEffects(core::registry::RegistryHub& registries)
	{
		registries.effects.add("battle.damage", std::make_unique<DamageEffect>());
		registries.effects.add("battle.heal", std::make_unique<HealEffect>());
		registries.effects.add("battle.move", std::make_unique<MoveEffect>());
		registries.effects.add("battle.set-action-state", std::make_unique<SetActionStateEffect>());
		registries.effects.add("battle.destroy-self", std::make_unique<DestroySelfEffect>());
	}
}
