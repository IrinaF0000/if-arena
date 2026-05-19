// Battle-owned turn loop and action rule execution.

#include "Features/Battle/Systems/BattleTurnSystem.hpp"

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Engine/ActionContext.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/ActionRuleParams.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Mutations/RemoveBattleEntityMutation.hpp"
#include "Features/Battle/Policies/BattlePolicyAccess.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <algorithm>
#include <exception>
#include <optional>
#include <string>
#include <vector>

namespace battle_sim::features::battle
{
	namespace
	{
		using core::ecs::ActionRule;
		using core::registry::HandlerId;

		HandlerId handlerId(const std::string& id)
		{
			return HandlerId::fromString(id);
		}

		HandlerId handlerOrDefault(const std::optional<HandlerId>& handler, const std::string& fallback)
		{
			return handler.value_or(handlerId(fallback));
		}

		bool hasCoreNoneSelector(const ActionRule& rule)
		{
			return !rule.selectorHandler.has_value() || rule.selectorHandler->str() == "core.none";
		}

		bool allowsEmptyTargets(const ActionRule& rule)
		{
			if (hasCoreNoneSelector(rule))
			{
				return true;
			}
			const auto it = rule.boolParams.find("allow-empty-targets");
			return it != rule.boolParams.end() && it->second;
		}

		int resolveValue(
			core::engine::ActionContext& actionContext,
			core::EntityId actor,
			const ActionRule& rule,
			const std::optional<HandlerId>& resolver,
			int fallback)
		{
			if (!resolver.has_value())
			{
				return fallback;
			}
			if (resolver->str() == "core.literal")
			{
				return fallback;
			}
			return actionContext.registries().valueResolvers.get(*resolver).resolve(actionContext, actor, rule);
		}

		int effectAmount(core::engine::ActionContext& actionContext, core::EntityId actor, const ActionRule& rule)
		{
			const auto id = rule.effectHandler.has_value() ? rule.effectHandler->str() : std::string{};
			if (id == "battle.move")
			{
				return resolveValue(actionContext, actor, rule, rule.stepDistanceValueResolver, rule.stepDistance);
			}
			return resolveValue(actionContext, actor, rule, rule.amountValueResolver, rule.amount);
		}

		bool requiresAttackableTarget(const ActionRule& rule)
		{
			const auto it = rule.boolParams.find(std::string{RequireAttackableTargetParam});
			return it != rule.boolParams.end() && it->second;
		}

		void filterAttackableTargets(
			const ActionRule& rule,
			const core::ecs::World& world,
			std::vector<core::EntityId>& targets)
		{
			if (!requiresAttackableTarget(rule))
			{
				return;
			}
			std::erase_if(targets, [&](core::EntityId target) { return !isAttackable(world, target); });
		}

		bool applyEffect(
			core::engine::ActionContext& actionContext,
			core::EntityId actor,
			const ActionRule& rule,
			const std::optional<HandlerId>& handler,
			const std::vector<core::EntityId>& targets,
			bool once)
		{
			if (!handler.has_value() || handler->str() == "core.none")
			{
				return false;
			}

			auto& effect = actionContext.registries().effects.get(*handler);
			const int amount = effectAmount(actionContext, actor, rule);
			if (once || targets.empty())
			{
				return effect.apply(actionContext, actor, 0, rule, amount);
			}

			bool applied = false;
			for (const auto target : targets)
			{
				applied = effect.apply(actionContext, actor, target, rule, amount) || applied;
			}
			return applied;
		}

		bool executeRule(core::engine::ActionContext& actionContext, core::EntityId actor, const ActionRule& rule)
		{
			auto& registries = actionContext.registries();
			auto& world = actionContext.world();

			if (!registries.conditions.get(handlerOrDefault(rule.conditionHandler, "core.always")).matches(actionContext, actor, rule))
			{
				return false;
			}

			auto targets = registries.targetSelectors.get(handlerOrDefault(rule.selectorHandler, "core.none")).select(
				actionContext,
				actor,
				rule);
			filterAttackableTargets(rule, world, targets);
			targets = registries.targetPickers.get(handlerOrDefault(rule.targetPickerHandler, "core.none")).pick(
				actionContext,
				actor,
				rule,
				targets);

			if (!allowsEmptyTargets(rule) && targets.empty())
			{
				return false;
			}

			const bool primaryApplied = applyEffect(actionContext, actor, rule, rule.effectHandler, targets, false);
			const bool afterApplied = applyEffect(actionContext, actor, rule, rule.afterEffectHandler, targets, true);
			return primaryApplied || afterApplied;
		}

		void cleanupDeadEntities(core::engine::GameContext& game)
		{
			auto& world = game.world;
			const auto& policies = battlePolicies(game);
			if (!shouldCleanupDeadAfterTick(policies))
			{
				return;
			}

			for (const auto entity : world.creationOrder())
			{
				if (!world.exists(entity) || isAlive(world, entity))
				{
					continue;
				}

				try
				{
					core::engine::MutationQueue::applyNow(
						game,
						makeRemoveBattleEntityMutation(entity, features::battle::RemovalReason::Death, false));
				}
				catch (const std::exception&)
				{
					continue;
				}
			}
		}
	}

	int BattleTurnSystem::executeEntity(core::engine::GameContext& game, core::EntityId entity)
	{
		auto& world = game.world;
		auto& components = world.components();
		auto& actionState = world.components<ActionStateComponent>();
		const auto* budget = components.actionBudget.get(entity);
		const auto* rules = components.actionRules.get(entity);
		if (!canActThisTurn(battlePolicies(game), world, entity) || !budget || !rules || budget->pointsPerTurn <= 0
			|| rules->rules.empty())
		{
			return 0;
		}
		if (!actionState.has(entity))
		{
			actionState.emplace(entity, ActionStateComponent{});
		}

		auto ordered = rules->rules;
		std::stable_sort(
			ordered.begin(),
			ordered.end(),
			[](const ActionRule& lhs, const ActionRule& rhs) { return lhs.priority > rhs.priority; });

		core::engine::ActionContext actionContext(game);
		int remainingBudget = budget->pointsPerTurn;
		int executed = 0;
		while (remainingBudget > 0)
		{
			bool progressed = false;
			for (const auto& rule : ordered)
			{
				if (rule.cost <= 0 || rule.cost > remainingBudget)
				{
					continue;
				}
				if (!executeRule(actionContext, entity, rule))
				{
					continue;
				}

				remainingBudget -= rule.cost;
				++executed;
				progressed = true;
				if (!world.exists(entity))
				{
					remainingBudget = 0;
				}
				break;
			}

			if (!progressed)
			{
				break;
			}
		}
		return executed;
	}

	int BattleTurnSystem::executeTick(core::engine::GameContext& game)
	{
		int tickActions = 0;
		for (const auto entity : game.world.creationOrder())
		{
			tickActions += executeEntity(game, entity);
		}
		return tickActions;
	}

	void BattleTurnSystem::cleanupDead(core::engine::GameContext& game)
	{
		cleanupDeadEntities(game);
	}

	bool BattleTurnSystem::canContinue(const core::engine::GameContext& game)
	{
		return game.resources.contains<BattleMapResource>() && hasEnoughActiveEntities(battlePolicies(game), game.world);
	}

	BattleTurnResult BattleTurnSystem::run(core::engine::GameContext& game)
	{
		if (!game.resources.contains<BattleMapResource>())
		{
			return {};
		}

		BattleTurnResult result;
		game.world.setTick(2);
		while (canContinue(game) && result.ticksExecuted < game.settings.maxTicks)
		{
			const int tickActions = executeTick(game);
			cleanupDeadEntities(game);
			result.actionsExecuted += tickActions;
			if (shouldStopAfterNoActions(battlePolicies(game), tickActions))
			{
				break;
			}

			++result.ticksExecuted;
			game.world.advanceTick();
		}
		return result;
	}
}
