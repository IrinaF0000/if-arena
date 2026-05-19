// Tests for feature-owned battle action execution.

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Engine/EngineRunner.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Core/Registry/CoreHandlers.hpp"
#include "Features/Battle/ActionRuleParams.hpp"
#include "Features/Battle/BattleFeaturePack.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/EntityRecipe.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"
#include "Features/Battle/Systems/BattleSpawnSystem.hpp"
#include "Features/Battle/Systems/BattleTurnSystem.hpp"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using battle_sim::core::registry::HandlerId;
	using battle_sim::features::battle::EntityRecipe;
	using battle_sim::features::battle::HealthComponent;
	using battle_sim::features::battle::PositionComponent;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	HandlerId handlerId(const std::string& id)
	{
		return HandlerId::fromString(id);
	}

	battle_sim::core::engine::GameContext makeBattleContext(int width = 4, int height = 4)
	{
		battle_sim::core::engine::GameContext game;
		battle_sim::core::registry::registerCoreHandlers(game.registries);
		battle_sim::features::battle::BattleFeaturePack{}.registerFeature(
			game,
			battle_sim::core::config::FeatureConfig{});
		game.resources.emplace<battle_sim::features::battle::BattleMapResource>(width, height);
		return game;
	}

	battle_sim::core::ecs::ActionRule adjacentDamageRule()
	{
		battle_sim::core::ecs::ActionRule rule;
		rule.conditionHandler = handlerId("core.always");
		rule.selectorHandler = handlerId("battle.adjacent-blocking");
		rule.targetPickerHandler = handlerId("core.any");
		rule.effectHandler = handlerId("battle.damage");
		rule.amountValueResolver = handlerId("battle.strength");
		rule.boolParams[std::string{battle_sim::features::battle::RequireAttackableTargetParam}] = true;
		rule.priority = 10;
		return rule;
	}

	battle_sim::core::ecs::ActionRule moveRule()
	{
		battle_sim::core::ecs::ActionRule rule;
		rule.conditionHandler = handlerId("battle.has-march-target");
		rule.selectorHandler = handlerId("core.none");
		rule.targetPickerHandler = handlerId("core.none");
		rule.effectHandler = handlerId("battle.move");
		rule.stepDistanceValueResolver = handlerId("battle.mobile-step");
		rule.priority = 5;
		return rule;
	}

	EntityRecipe unit(
		battle_sim::core::EntityId id,
		battle_sim::core::Position position,
		int hp,
		int strength,
		std::vector<battle_sim::core::ecs::ActionRule> rules)
	{
		EntityRecipe recipe;
		recipe.id = id;
		recipe.position = position;
		recipe.stats.health.hp = hp;
		recipe.stats.strength.value = strength;
		recipe.actionBudget.pointsPerTurn = 1;
		recipe.actionRules.rules = std::move(rules);
		return recipe;
	}

	void adjacentDamageIsExecutedByBattleTurnSystem()
	{
		auto game = makeBattleContext();
		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(unit(1, {0, 0}, 10, 3, {adjacentDamageRule()}), game, 1),
			"Expected attacker spawn");
		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(unit(2, {1, 0}, 3, 1, {}), game, 1),
			"Expected target spawn");

		const auto executed = battle_sim::features::battle::BattleTurnSystem::executeEntity(game, 1);
		require(executed == 1, "Expected one executed action");
		require(game.world.components<HealthComponent>().get(2)->hp == 0, "Expected damage to reduce target health");
		require(!game.world.components().alive.has(2), "Expected dead target to lose alive marker");
	}

	void movementUsesBattleMarchTargetComponent()
	{
		auto game = makeBattleContext(5, 1);
		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(unit(1, {0, 0}, 10, 1, {moveRule()}), game, 1),
			"Expected mover spawn");
		game.world.components<battle_sim::features::battle::MarchTargetComponent>().emplace(1, {{2, 0}});

		const auto executed = battle_sim::features::battle::BattleTurnSystem::executeEntity(game, 1);
		require(executed == 1, "Expected one movement action");
		require(
			game.world.components<PositionComponent>().get(1)->value == battle_sim::core::Position{1, 0},
			"Expected movement toward march target");
	}

	void schedulerPathRunsBattleTick()
	{
		auto game = makeBattleContext();
		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(unit(1, {0, 0}, 10, 3, {adjacentDamageRule()}), game, 1),
			"Expected attacker spawn");
		require(
			battle_sim::features::battle::BattleSpawnSystem::spawn(unit(2, {1, 0}, 3, 1, {}), game, 1),
			"Expected target spawn");

		const auto result = battle_sim::core::engine::EngineRunner{game}.run();

		require(result.ticksExecuted == 1, "Expected scheduler to execute one battle tick");
		require(!game.world.components<HealthComponent>().has(2), "Expected scheduler cleanup to remove target health");
		require(!game.world.exists(2), "Expected scheduler cleanup to remove dead target");
	}
}

int main()
{
	const std::vector<std::pair<std::string, void (*)()>> tests{
		{"adjacentDamageIsExecutedByBattleTurnSystem", adjacentDamageIsExecutedByBattleTurnSystem},
		{"movementUsesBattleMarchTargetComponent", movementUsesBattleMarchTargetComponent},
		{"schedulerPathRunsBattleTick", schedulerPathRunsBattleTick},
	};

	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
		}
		catch (const std::exception& error)
		{
			std::cerr << name << " failed: " << error.what() << '\n';
			return 1;
		}
	}

	std::cout << "ecs_action_executor_tests: " << tests.size() << " passed\n";
	return 0;
}
