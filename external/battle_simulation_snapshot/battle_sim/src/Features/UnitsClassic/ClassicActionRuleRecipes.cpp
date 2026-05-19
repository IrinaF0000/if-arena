// Classic unit behavior recipes expressed as handler-dispatched ECS action rules.

#include "Features/UnitsClassic/ClassicActionRuleRecipes.hpp"

#include "Core/Ecs/ActionRules.hpp"
#include "Features/Battle/ActionRuleParams.hpp"
#include "Core/Registry/HandlerId.hpp"

#include <optional>
#include <string>
#include <utility>

namespace battle_sim::features::units_classic
{
	namespace
	{
		using battle_sim::core::ecs::ActionRule;
		using battle_sim::core::ecs::ActionRulesComponent;
		using battle_sim::core::registry::HandlerId;

		HandlerId handlerId(std::string value)
		{
			return HandlerId::fromString(std::move(value));
		}

		void setHandler(std::optional<HandlerId>& target, const std::string& value)
		{
			target = handlerId(value);
		}

		ActionRule damage(
			int priority,
			const std::string& selector,
			const std::string& picker,
			const std::string& amountResolver,
			int minDistance,
			int maxDistance,
			std::optional<std::string> maxDistanceResolver = std::nullopt,
			bool restrictToAttackableTargets = false)
		{
			ActionRule rule;
			setHandler(rule.conditionHandler, "core.always");
			setHandler(rule.selectorHandler, selector);
			setHandler(rule.targetPickerHandler, picker);
			setHandler(rule.effectHandler, "battle.damage");
			setHandler(rule.amountValueResolver, amountResolver);
			if (maxDistanceResolver.has_value())
			{
				setHandler(rule.maxDistanceValueResolver, *maxDistanceResolver);
			}
			rule.minDistance = minDistance;
			rule.maxDistance = maxDistance;
			if (restrictToAttackableTargets)
			{
				rule.boolParams[std::string{battle_sim::features::battle::RequireAttackableTargetParam}] = true;
			}
			rule.priority = priority;
			return rule;
		}

		ActionRule move(int priority, int stepDistance)
		{
			ActionRule rule;
			setHandler(rule.conditionHandler, "core.always");
			setHandler(rule.selectorHandler, "core.none");
			setHandler(rule.targetPickerHandler, "core.none");
			setHandler(rule.effectHandler, "battle.move");
			setHandler(rule.stepDistanceValueResolver, "core.literal");
			rule.stepDistance = stepDistance;
			rule.priority = priority;
			return rule;
		}
	}

	core::ecs::ActionRulesComponent makeSwordsmanActionRules()
	{
		ActionRulesComponent rules;
		rules.rules.push_back(damage(
			20,
			"battle.adjacent-blocking",
			"core.random-one",
			"battle.strength",
			1,
			1));
		rules.rules.push_back(move(10, 1));
		return rules;
	}

	core::ecs::ActionRulesComponent makeLancerActionRules()
	{
		ActionRulesComponent rules;
		rules.rules.push_back(damage(
			30,
			"battle.effective-range",
			"core.random-one",
			"battle.strength",
			2,
			2,
			std::nullopt,
			true));
		rules.rules.push_back(damage(
			20,
			"battle.adjacent-blocking",
			"core.random-one",
			"battle.strength",
			1,
			1));
		rules.rules.push_back(move(10, 1));
		return rules;
	}

	core::ecs::ActionRulesComponent makeHunterActionRules()
	{
		ActionRulesComponent rules;
		auto shot = damage(
			30,
			"battle.effective-range",
			"core.random-one",
			"battle.agility",
			2,
			0,
			"battle.range",
			true);
		setHandler(shot.conditionHandler, "battle.no-adjacent-blocking");
		rules.rules.push_back(shot);
		rules.rules.push_back(damage(
			20,
			"battle.adjacent-blocking",
			"core.random-one",
			"battle.strength",
			1,
			1));
		rules.rules.push_back(move(10, 1));
		return rules;
	}

	core::ecs::ActionRulesComponent makeTowerActionRules()
	{
		ActionRulesComponent rules;
		rules.rules.push_back(damage(
			10,
			"battle.effective-range",
			"core.random-one",
			"battle.power",
			2,
			5,
			std::nullopt,
			true));
		return rules;
	}

	core::ecs::ActionRulesComponent makeRavenActionRules()
	{
		ActionRulesComponent rules;
		rules.rules.push_back(damage(
			20,
			"battle.adjacent-blocking",
			"core.random-one",
			"battle.agility",
			1,
			1));
		rules.rules.push_back(move(10, 2));
		return rules;
	}

	core::ecs::ActionRulesComponent makeHealerActionRules()
	{
		ActionRulesComponent rules;
		ActionRule heal;
		setHandler(heal.conditionHandler, "core.always");
		setHandler(heal.selectorHandler, "battle.effective-range");
		setHandler(heal.targetPickerHandler, "core.random-one");
		setHandler(heal.effectHandler, "battle.heal");
		setHandler(heal.amountValueResolver, "battle.spirit");
		heal.maxDistance = 2;
		heal.boolParams[std::string{battle_sim::features::battle::RequireAttackableTargetParam}] = true;
		heal.priority = 20;
		rules.rules.push_back(heal);
		rules.rules.push_back(move(10, 1));
		return rules;
	}

	core::ecs::ActionRulesComponent makeMineActionRules()
	{
		ActionRulesComponent rules;

		ActionRule explode;
		setHandler(explode.conditionHandler, "battle.has-action-state");
		setHandler(explode.selectorHandler, "battle.physical-radius");
		setHandler(explode.targetPickerHandler, "core.all");
		setHandler(explode.effectHandler, "battle.damage");
		setHandler(explode.afterEffectHandler, "battle.destroy-self");
		setHandler(explode.amountValueResolver, "battle.power");
		explode.maxDistance = 3;
		explode.stateKey = "armed";
		explode.priority = 20;
		explode.boolParams["allow-empty-targets"] = true;
		rules.rules.push_back(explode);

		ActionRule arm;
		setHandler(arm.conditionHandler, "battle.missing-action-state");
		setHandler(arm.selectorHandler, "battle.physical-radius");
		setHandler(arm.targetPickerHandler, "core.any");
		setHandler(arm.effectHandler, "battle.set-action-state");
		arm.maxDistance = 2;
		arm.stateKey = "armed";
		arm.stateValue = 1;
		arm.priority = 10;
		rules.rules.push_back(arm);

		return rules;
	}
}
