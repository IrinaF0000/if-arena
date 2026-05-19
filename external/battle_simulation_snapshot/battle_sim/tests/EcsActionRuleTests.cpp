// Tests for the data-only ECS action rule DSL.

#include "Core/Ecs/ActionRules.hpp"
#include "Features/UnitsClassic/ClassicActionRuleRecipes.hpp"
#include "Features/Battle/ActionRuleParams.hpp"

#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using battle_sim::core::ecs::ActionBudgetComponent;
	using battle_sim::core::ecs::ActionRule;
	using battle_sim::core::ecs::ActionRulesComponent;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void requireHandler(
		const std::optional<battle_sim::core::registry::HandlerId>& actual,
		const std::string& expected,
		const std::string& message)
	{
		require(actual.has_value(), message + ": missing handler id");
		require(actual->str() == expected, message + ": expected " + expected + ", got " + actual->str());
	}

	battle_sim::core::registry::HandlerId handlerId(const std::string& value)
	{
		return battle_sim::core::registry::HandlerId::fromString(value);
	}

	ActionRule damage(
		int priority,
		const std::string& selector,
		const std::string& picker,
		const std::string& amountResolver,
		int minDistance,
		int maxDistance,
		bool restrictToAttackableTargets = false)
	{
		ActionRule rule;
		rule.conditionHandler = handlerId("core.always");
		rule.selectorHandler = handlerId(selector);
		rule.targetPickerHandler = handlerId(picker);
		rule.effectHandler = handlerId("battle.damage");
		rule.amountValueResolver = handlerId(amountResolver);
		rule.minDistance = minDistance;
		rule.maxDistance = maxDistance;
		if (restrictToAttackableTargets)
		{
			rule.boolParams[std::string{battle_sim::features::battle::RequireAttackableTargetParam}] = true;
		}
		rule.priority = priority;
		return rule;
	}

	ActionRule move(int priority, int stepDistance = 1)
	{
		ActionRule rule;
		rule.conditionHandler = handlerId("core.always");
		rule.selectorHandler = handlerId("core.none");
		rule.targetPickerHandler = handlerId("core.none");
		rule.effectHandler = handlerId("battle.move");
		rule.stepDistanceValueResolver = handlerId("core.literal");
		rule.stepDistance = stepDistance;
		rule.priority = priority;
		return rule;
	}

	void currentArchetypeBehaviorsAreExpressibleAsRules()
	{
		ActionRulesComponent swordsman;
		swordsman.rules.push_back(damage(20, "battle.adjacent-blocking", "core.random-one", "battle.strength", 1, 1));
		swordsman.rules.push_back(move(10));

		ActionRulesComponent lancer;
		lancer.rules.push_back(damage(30, "battle.effective-range", "core.random-one", "battle.strength", 2, 2, true));
		lancer.rules.push_back(damage(20, "battle.adjacent-blocking", "core.random-one", "battle.strength", 1, 1));
		lancer.rules.push_back(move(10));

		ActionRulesComponent hunter;
		ActionRule hunterShot =
			damage(30, "battle.effective-range", "core.random-one", "battle.agility", 2, 0, true);
		hunterShot.conditionHandler = handlerId("battle.no-adjacent-blocking");
		hunterShot.maxDistanceValueResolver = handlerId("battle.range");
		hunter.rules.push_back(hunterShot);
		hunter.rules.push_back(damage(20, "battle.adjacent-blocking", "core.random-one", "battle.strength", 1, 1));
		hunter.rules.push_back(move(10));

		ActionRulesComponent tower;
		tower.rules.push_back(damage(10, "battle.effective-range", "core.random-one", "battle.power", 2, 5, true));

		ActionRulesComponent raven;
		raven.rules.push_back(damage(20, "battle.adjacent-blocking", "core.random-one", "battle.agility", 1, 1));
		raven.rules.push_back(move(10, 2));

		ActionRulesComponent healer;
		ActionRule heal;
		heal.conditionHandler = handlerId("core.always");
		heal.selectorHandler = handlerId("battle.effective-range");
		heal.targetPickerHandler = handlerId("core.random-one");
		heal.effectHandler = handlerId("battle.heal");
		heal.amountValueResolver = handlerId("battle.spirit");
		heal.maxDistance = 2;
		heal.boolParams[std::string{battle_sim::features::battle::RequireAttackableTargetParam}] = true;
		heal.priority = 20;
		healer.rules.push_back(heal);
		healer.rules.push_back(move(10));

		ActionRulesComponent mine;
		ActionRule explode;
		explode.conditionHandler = handlerId("battle.has-action-state");
		explode.selectorHandler = handlerId("battle.physical-radius");
		explode.targetPickerHandler = handlerId("core.all");
		explode.effectHandler = handlerId("battle.damage");
		explode.afterEffectHandler = handlerId("battle.destroy-self");
		explode.amountValueResolver = handlerId("battle.power");
		explode.maxDistance = 3;
		explode.stateKey = "armed";
		explode.boolParams["allow-empty-targets"] = true;
		explode.priority = 20;
		mine.rules.push_back(explode);

		ActionRule arm;
		arm.conditionHandler = handlerId("battle.missing-action-state");
		arm.selectorHandler = handlerId("battle.physical-radius");
		arm.targetPickerHandler = handlerId("core.any");
		arm.effectHandler = handlerId("battle.set-action-state");
		arm.maxDistance = 2;
		arm.stateKey = "armed";
		arm.stateValue = 1;
		arm.priority = 10;
		mine.rules.push_back(arm);

		require(swordsman.rules.size() == 2, "Expected swordsman to have attack and move rules");
		require(lancer.rules.size() == 3, "Expected lancer to have reach, melee, and move rules");
		require(lancer.rules[0].boolParams.at(std::string{battle_sim::features::battle::RequireAttackableTargetParam}), "Expected lancer reach to require attackable targets");
		require(lancer.rules[0].minDistance == 2 && lancer.rules[0].maxDistance == 2, "Expected lancer reach distance two");
		require(hunter.rules.size() == 3, "Expected hunter to have shot, melee, and move rules");
		require(swordsman.rules[0].boolParams.find(std::string{battle_sim::features::battle::RequireAttackableTargetParam}) == swordsman.rules[0].boolParams.end(), "Expected swordsman melee to match adjacent-alive behavior");
		require(hunter.rules[0].boolParams.at(std::string{battle_sim::features::battle::RequireAttackableTargetParam}), "Expected hunter shot to require attackable targets");
		require(hunter.rules[1].boolParams.find(std::string{battle_sim::features::battle::RequireAttackableTargetParam}) == hunter.rules[1].boolParams.end(), "Expected hunter melee to match adjacent-alive behavior");
		require(tower.rules.size() == 1, "Expected tower to have one ranged power rule");
		require(tower.rules[0].boolParams.at(std::string{battle_sim::features::battle::RequireAttackableTargetParam}), "Expected tower shot to require attackable targets");
		requireHandler(raven.rules[0].amountValueResolver, "battle.agility", "Expected raven melee to use agility");
		require(raven.rules[0].boolParams.find(std::string{battle_sim::features::battle::RequireAttackableTargetParam}) == raven.rules[0].boolParams.end(), "Expected raven melee to match adjacent-alive behavior");
		require(raven.rules[1].stepDistance == 2, "Expected raven move to use a two-cell step");
		requireHandler(healer.rules[0].effectHandler, "battle.heal", "Expected healer rule to heal");
		requireHandler(healer.rules[0].amountValueResolver, "battle.spirit", "Expected healer rule to use spirit");
		requireHandler(mine.rules[0].targetPickerHandler, "core.all", "Expected mine explosion to hit all physical-radius targets");
		requireHandler(mine.rules[0].afterEffectHandler, "battle.destroy-self", "Expected mine explosion to destroy self after damage");
		requireHandler(mine.rules[1].conditionHandler, "battle.missing-action-state", "Expected mine arm rule to depend on missing state");
	}

	void actionBudgetCanDescribeMultipleActionsPerTurn()
	{
		ActionBudgetComponent budget{2};
		ActionRulesComponent rules;
		rules.rules.push_back(damage(20, "battle.adjacent-blocking", "core.random-one", "battle.strength", 1, 1));
		rules.rules.push_back(move(10));

		require(budget.pointsPerTurn == 2, "Expected action budget to allow more than one action");
		require(rules.rules.size() == 2, "Expected two rules for a two-action archetype");
		require(rules.rules[0].cost == 1, "Expected default rule cost to be one budget point");
		require(rules.rules[1].cost == 1, "Expected move cost to be one budget point");
	}

	void currentRecipesCarryHandlerIds()
	{
		const auto swordsman = battle_sim::features::units_classic::makeSwordsmanActionRules();
		requireHandler(swordsman.rules[0].conditionHandler, "core.always", "Expected swordsman condition handler");
		requireHandler(swordsman.rules[0].selectorHandler, "battle.adjacent-blocking", "Expected swordsman selector handler");
		requireHandler(swordsman.rules[0].targetPickerHandler, "core.random-one", "Expected swordsman picker handler");
		requireHandler(swordsman.rules[0].effectHandler, "battle.damage", "Expected swordsman effect handler");
		requireHandler(swordsman.rules[0].amountValueResolver, "battle.strength", "Expected swordsman value resolver");
		requireHandler(swordsman.rules[1].selectorHandler, "core.none", "Expected swordsman move selector handler");
		requireHandler(swordsman.rules[1].effectHandler, "battle.move", "Expected swordsman move effect handler");
		requireHandler(swordsman.rules[1].stepDistanceValueResolver, "core.literal", "Expected swordsman move step resolver");

		const auto lancer = battle_sim::features::units_classic::makeLancerActionRules();
		requireHandler(lancer.rules[0].selectorHandler, "battle.effective-range", "Expected lancer reach selector handler");
		requireHandler(lancer.rules[0].amountValueResolver, "battle.strength", "Expected lancer reach to use strength");
		require(lancer.rules[0].minDistance == 2 && lancer.rules[0].maxDistance == 2, "Expected lancer reach distance");
		requireHandler(lancer.rules[1].selectorHandler, "battle.adjacent-blocking", "Expected lancer melee fallback");
		requireHandler(lancer.rules[2].effectHandler, "battle.move", "Expected lancer move fallback");

		const auto hunter = battle_sim::features::units_classic::makeHunterActionRules();
		requireHandler(hunter.rules[0].conditionHandler, "battle.no-adjacent-blocking", "Expected hunter shot condition handler");
		requireHandler(hunter.rules[0].selectorHandler, "battle.effective-range", "Expected hunter shot selector handler");
		requireHandler(hunter.rules[0].maxDistanceValueResolver, "battle.range", "Expected hunter shot range resolver");

		const auto tower = battle_sim::features::units_classic::makeTowerActionRules();
		requireHandler(tower.rules[0].amountValueResolver, "battle.power", "Expected tower power resolver");

		const auto raven = battle_sim::features::units_classic::makeRavenActionRules();
		requireHandler(raven.rules[0].amountValueResolver, "battle.agility", "Expected raven agility resolver");

		const auto healer = battle_sim::features::units_classic::makeHealerActionRules();
		requireHandler(healer.rules[0].effectHandler, "battle.heal", "Expected healer effect handler");
		requireHandler(healer.rules[0].amountValueResolver, "battle.spirit", "Expected healer value resolver");

		const auto mine = battle_sim::features::units_classic::makeMineActionRules();
		requireHandler(mine.rules[0].conditionHandler, "battle.has-action-state", "Expected mine explosion condition handler");
		requireHandler(mine.rules[0].selectorHandler, "battle.physical-radius", "Expected mine explosion selector handler");
		requireHandler(mine.rules[0].targetPickerHandler, "core.all", "Expected mine explosion picker handler");
		requireHandler(mine.rules[0].effectHandler, "battle.damage", "Expected mine explosion effect handler");
		requireHandler(mine.rules[0].afterEffectHandler, "battle.destroy-self", "Expected mine explosion after-effect handler");
		requireHandler(mine.rules[0].amountValueResolver, "battle.power", "Expected mine explosion value resolver");
		require(mine.rules[0].boolParams.at("allow-empty-targets"), "Expected mine explosion to allow self-destruction without targets");
		requireHandler(mine.rules[1].conditionHandler, "battle.missing-action-state", "Expected mine arm condition handler");
		requireHandler(mine.rules[1].targetPickerHandler, "core.any", "Expected mine arm picker handler");
		requireHandler(mine.rules[1].effectHandler, "battle.set-action-state", "Expected mine arm effect handler");
	}

	void actionRulesCarryGenericHandlerParams()
	{
		ActionRule rule;
		rule.intParams.emplace("limit", 3);
		rule.boolParams.emplace("enabled", true);
		rule.stringParams.emplace("mode", "strict");

		require(rule.intParams.at("limit") == 3, "Expected integer handler parameter");
		require(rule.boolParams.at("enabled"), "Expected boolean handler parameter");
		require(rule.stringParams.at("mode") == "strict", "Expected string handler parameter");
		require(rule.intParams.find("range") == rule.intParams.end(), "Expected params to stay generic by key");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"currentArchetypeBehaviorsAreExpressibleAsRules", currentArchetypeBehaviorsAreExpressibleAsRules},
		{"actionBudgetCanDescribeMultipleActionsPerTurn", actionBudgetCanDescribeMultipleActionsPerTurn},
		{"currentRecipesCarryHandlerIds", currentRecipesCarryHandlerIds},
		{"actionRulesCarryGenericHandlerParams", actionRulesCarryGenericHandlerParams},
	};

	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
			std::cout << "[PASS] " << name << '\n';
		}
		catch (const std::exception& ex)
		{
			++failed;
			std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
		}
	}

	if (failed != 0)
	{
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}

	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
