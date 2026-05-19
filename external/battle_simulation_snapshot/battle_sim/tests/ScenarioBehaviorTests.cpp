// Scenario-level behavior tests for the ECS-style battle runtime.

#include "App/ScenarioConfig.hpp"
#include "App/ScenarioRunner.hpp"
#include "App/TraceInspector.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Core/Registry/CoreHandlers.hpp"
#include "Features/Battle/BattleFeaturePack.hpp"
#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/UnitsClassic/DataDrivenArchetypes.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	struct CapturedScenario
	{
		std::string output;
		std::string error;
	};

	CapturedScenario runScenarioCaptured(const std::string& scenario, battle_sim::app::RunOptions options = {})
	{
		std::istringstream input(scenario);
		std::ostringstream output;
		std::ostringstream error;
		auto* previousError = std::cerr.rdbuf(error.rdbuf());
		try
		{
			battle_sim::app::runScenario(input, output, options);
			std::cerr.rdbuf(previousError);
		}
		catch (...)
		{
			std::cerr.rdbuf(previousError);
			throw;
		}
		return CapturedScenario{output.str(), error.str()};
	}

	std::string runScenarioText(const std::string& scenario, battle_sim::app::RunOptions options = {})
	{
		return runScenarioCaptured(scenario, options).output;
	}

	void requireContains(const std::string& text, const std::string& expected)
	{
		if (text.find(expected) == std::string::npos)
		{
			throw std::runtime_error("Expected output to contain: " + expected + "\nActual output:\n" + text);
		}
	}

	void requireEqual(const std::string& actual, const std::string& expected, const std::string& label)
	{
		if (actual != expected)
		{
			throw std::runtime_error("Expected " + label + " to match.\nExpected:\n" + expected + "\nActual:\n" + actual);
		}
	}

	void requireNotContains(const std::string& text, const std::string& unexpected)
	{
		if (text.find(unexpected) != std::string::npos)
		{
			throw std::runtime_error("Expected output not to contain: " + unexpected + "\nActual output:\n" + text);
		}
	}

	void requireInOrder(const std::string& text, const std::string& first, const std::string& second)
	{
		const auto firstPosition = text.find(first);
		const auto secondPosition = text.find(second);
		if (firstPosition == std::string::npos || secondPosition == std::string::npos || firstPosition >= secondPosition)
		{
			throw std::runtime_error(
				"Expected output to contain entries in order:\n1. " + first + "\n2. " + second + "\nActual output:\n" + text);
		}
	}

	void swordsmanAttacksAdjacent()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 3\n"
			"SPAWN_SWORDSMAN 1 0 0 5 3\n"
			"SPAWN_SWORDSMAN 2 1 0 3 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=0");
		requireContains(output, "[2] UNIT_DIED unitId=2");
	}

	void swordsmanMovesWhenNoAdjacentTarget()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN_SWORDSMAN 1 0 0 10 1\n"
			"SPAWN_MINE 2 3 0 1\n"
			"MARCH 1 1 0\n");

		requireContains(output, "[2] UNIT_MOVED unitId=1 x=1 y=0");
	}

	void mobileEntityWithoutMarchDoesNotWander()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_SWORDSMAN 1 0 0 10 1\n"
			"SPAWN_SWORDSMAN 2 4 0 10 1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Swordsman x=0 y=0");
		requireContains(output, "[1] UNIT_SPAWNED unitId=2 unitType=Swordsman x=4 y=0");
		requireNotContains(output, "UNIT_MOVED");
		requireNotContains(output, "UNIT_ATTACKED");
	}

	void hunterShootsAtRange()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 5\n"
			"SPAWN_HUNTER 1 0 0 10 4 1 3\n"
			"SPAWN_TOWER 2 2 0 4 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=4 targetHp=0");
		requireContains(output, "[2] UNIT_DIED unitId=2");
	}

	void hunterShootsAtMaxRange()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_HUNTER 1 0 0 10 4 1 3\n"
			"SPAWN_TOWER 2 3 0 5 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=4 targetHp=1");
	}

	void hunterMovesWhenTargetBeyondRange()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_HUNTER 1 0 0 10 4 1 2\n"
			"SPAWN_MINE 2 4 0 1\n"
			"MARCH 1 1 0\n");

		requireContains(output, "[2] UNIT_MOVED unitId=1 x=1 y=0");
		requireNotContains(output, "UNIT_ATTACKED attackerUnitId=1");
	}

	void hunterUsesMeleeWhenAdjacentCellOccupied()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_HUNTER 1 0 0 10 9 2 5\n"
			"SPAWN_SWORDSMAN 2 1 0 5 1\n"
			"SPAWN_TOWER 3 3 0 5 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=2 targetHp=3");
		requireNotContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=3 damage=9");
	}

	void towerUsesPower()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 5\n"
			"SPAWN_TOWER 1 0 0 10 3\n"
			"SPAWN_SWORDSMAN 2 2 0 5 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=2");
	}

	void towerShootsAtMaxRange()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 6 1\n"
			"SPAWN_TOWER 1 0 0 10 4\n"
			"SPAWN_SWORDSMAN 2 5 0 5 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=4 targetHp=1");
	}

	void towerDoesNotMoveWithoutTarget()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_TOWER 1 0 0 10 3\n"
			"SPAWN_MINE 2 1 0 1\n");

		requireNotContains(output, "UNIT_MOVED unitId=1");
		requireNotContains(output, "UNIT_ATTACKED attackerUnitId=1");
	}

	void simulationStopsWhenNoEntityCanAct()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 10 1\n"
			"SPAWN_TOWER 1 0 0 10 3\n"
			"SPAWN_TOWER 2 9 0 10 3\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Tower x=0 y=0");
		requireContains(output, "[1] UNIT_SPAWNED unitId=2 unitType=Tower x=9 y=0");
		requireNotContains(output, "UNIT_MOVED");
		requireNotContains(output, "UNIT_ATTACKED");
	}

	void ravenMovesTwoCells()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_RAVEN 1 0 0 10 2\n"
			"SPAWN_MINE 2 4 0 5\n"
			"MARCH 1 2 0\n");

		requireContains(output, "[2] UNIT_MOVED unitId=1 x=2 y=0");
	}

	void marchEndedReportsFinalPosition()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_RAVEN 1 0 0 10 2\n"
			"SPAWN_MINE 2 4 0 5\n"
			"MARCH 1 2 0\n");

		requireContains(output, "[2] MARCH_ENDED unitId=1 x=2 y=0");
	}

	void ravenAllowsBlockingEntityOnSameCell()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_RAVEN 1 0 0 1 1\n"
			"SPAWN_MINE 2 0 0 1\n"
			"SPAWN_SWORDSMAN 3 0 0 1 1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Raven x=0 y=0");
		requireContains(output, "[1] UNIT_SPAWNED unitId=3 unitType=Swordsman x=0 y=0");
	}

	void ravenAttacksAdjacentWithAgility()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_RAVEN 1 0 0 10 4\n"
			"SPAWN_SWORDSMAN 2 1 0 4 1\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=4 targetHp=0");
		requireContains(output, "[2] UNIT_DIED unitId=2");
	}

	void ravenCanBeShotAtEffectiveRange()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_TOWER 1 0 0 10 3\n"
			"SPAWN_RAVEN 2 1 0 3 0\n");

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=0");
		requireContains(output, "[2] UNIT_DIED unitId=2");
	}

	void healerRestoresHealth()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN_SWORDSMAN 1 0 0 10 2\n"
			"SPAWN_HEALER 2 3 0 1 2\n"
			"SPAWN_TOWER 3 1 0 3 1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=2 unitType=Healer x=3 y=0");
		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=3 damage=2 targetHp=1");
		requireContains(output, "[2] UNIT_DIED unitId=2");
		requireContains(output, "[3] UNIT_ATTACKED attackerUnitId=1 targetUnitId=3 damage=2 targetHp=1");
	}

	void healerIgnoresNonAttackableMineAndMoves()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN_HEALER 1 0 0 10 2\n"
			"SPAWN_MINE 2 1 0 1\n"
			"MARCH 1 2 0\n");

		requireContains(output, "[2] UNIT_MOVED unitId=1 x=1 y=0");
	}

	void healerMovesWhenNoHealTarget()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN_HEALER 1 0 0 10 2\n"
			"SPAWN_MINE 2 3 0 1\n"
			"MARCH 1 1 0\n");

		requireContains(output, "[2] UNIT_MOVED unitId=1 x=1 y=0");
	}

	void mineAllowsBlockingEntityOnSameCell()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_MINE 1 0 0 1\n"
			"SPAWN_SWORDSMAN 2 0 0 1 1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Mine x=0 y=0");
		requireContains(output, "[1] UNIT_SPAWNED unitId=2 unitType=Swordsman x=0 y=0");
	}

	void mineDoesNotMove()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN_MINE 1 0 0 1\n"
			"MARCH 1 1 0\n"
			"SPAWN_SWORDSMAN 2 2 0 5 1\n");

		requireNotContains(output, "UNIT_MOVED unitId=1");
		requireNotContains(output, "MARCH_STARTED unitId=1");
	}

	void mineIsNotHitByTargetedAttack()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN_SWORDSMAN 1 0 0 10 5\n"
			"SPAWN_MINE 2 1 0 1\n");

		requireNotContains(output, "UNIT_ATTACKED attackerUnitId=1 targetUnitId=2");
	}

	void fatallyDamagedEntityFinishesCurrentTurnOnly()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 6 1\n"
			"SPAWN_SWORDSMAN 1 0 0 10 5\n"
			"SPAWN_SWORDSMAN 2 1 0 1 2\n"
			"SPAWN_TOWER 3 5 0 10 1\n");

		requireInOrder(
			output,
			"[2] UNIT_DIED unitId=2",
			"[2] UNIT_ATTACKED attackerUnitId=2 targetUnitId=1 damage=2 targetHp=8");
		requireNotContains(output, "[3] UNIT_ATTACKED attackerUnitId=2");
	}

	void mineExplodesAndDisappears()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 5\n"
			"SPAWN_MINE 1 1 1 2\n"
			"SPAWN_MINE 2 2 1 2\n"
			"SPAWN_SWORDSMAN 3 1 2 5 1\n");

		requireContains(output, "[3] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=2 targetHp=0");
		requireContains(output, "[3] UNIT_DIED unitId=2");
		requireContains(output, "[3] UNIT_DIED unitId=1");
	}

	void mineExplosionUsesPhysicalRadiusThree()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 5 1\n"
			"SPAWN_MINE 1 0 0 2\n"
			"SPAWN_MINE 2 2 0 1\n"
			"SPAWN_MINE 3 3 0 1\n"
			"SPAWN_TOWER 4 4 0 5 1\n");

		requireContains(output, "[3] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=2 targetHp=0");
		requireContains(output, "[3] UNIT_ATTACKED attackerUnitId=1 targetUnitId=3 damage=2 targetHp=0");
		requireNotContains(output, "attackerUnitId=1 targetUnitId=4 damage=2");
		requireContains(output, "[3] UNIT_DIED unitId=1");
	}

	void fixedSeedReplaysRandomChoices()
	{
		const std::string scenario =
			"CREATE_MAP 3 3\n"
			"SPAWN_SWORDSMAN 1 1 1 10 1\n"
			"SPAWN_SWORDSMAN 2 0 1 1 1\n"
			"SPAWN_SWORDSMAN 3 2 1 1 1\n";

		battle_sim::app::RunOptions options;
		options.rngSeed = 123u;

		const auto firstRun = runScenarioText(scenario, options);
		const auto secondRun = runScenarioText(scenario, options);

		if (firstRun != secondRun)
		{
			throw std::runtime_error("Expected fixed seed to replay the same event log");
		}
		requireContains(firstRun, "UNIT_ATTACKED attackerUnitId=1");
	}

	void configSeedReplaysRandomChoices()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic\n"
			"feature io.legacy\n"
			"rngSeed 123\n");
		const auto options = battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));

		const std::string scenario =
			"CREATE_MAP 3 3\n"
			"SPAWN_SWORDSMAN 1 1 1 10 1\n"
			"SPAWN_SWORDSMAN 2 0 1 1 1\n"
			"SPAWN_SWORDSMAN 3 2 1 1 1\n";

		const auto firstRun = runScenarioText(scenario, options);
		const auto secondRun = runScenarioText(scenario, options);

		if (firstRun != secondRun)
		{
			throw std::runtime_error("Expected config seed to replay the same event log");
		}
	}

	void configRejectsUnknownFeature()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic\n"
			"feature io.legacy\n"
			"feature missing.pack\n");

		try
		{
			battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));
		}
		catch (const std::runtime_error& ex)
		{
			requireContains(ex.what(), "Unknown feature pack in config: missing.pack");
			return;
		}

		throw std::runtime_error("Expected unknown feature pack to be rejected");
	}

	void configRejectsDisabledRequiredFeature()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic enabled=false\n"
			"feature io.legacy\n");

		try
		{
			battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));
		}
		catch (const std::runtime_error& ex)
		{
			requireContains(ex.what(), "Required feature disabled or missing: units.classic");
			return;
		}

		throw std::runtime_error("Expected disabled required feature pack to be rejected");
	}

	void dataDrivenArchetypeLoadsFromConfig()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic\n"
			"feature io.legacy\n"
			"archetypeFile config/archetypes/duelist.json\n");
		const auto options = battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));

		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN Duelist 1 0 0\n"
			"SPAWN Swordsman 2 1 0 hp=3 strength=1\n",
			options);

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Duelist x=0 y=0");
		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=2 targetHp=1");
	}

	void jsonRuleArrayDefinesSpearman()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic\n"
			"feature io.legacy\n"
			"archetypeFile config/archetypes/spearman.json\n");
		const auto options = battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));

		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN Spearman 1 0 0\n"
			"SPAWN Tower 2 2 0 hp=5 power=1\n",
			options);

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Spearman x=0 y=0");
		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=2 targetHp=3");
		requireNotContains(output, "UNIT_MOVED unitId=1");
	}

	void jsonRuleArrayRejectsUnknownHandler()
	{
		battle_sim::core::engine::GameContext game;
		battle_sim::core::registry::registerCoreHandlers(game.registries);
		battle_sim::features::battle::BattleFeaturePack{}.registerFeature(
			game,
			battle_sim::core::config::FeatureConfig{});
		battle_sim::features::battle::EntityArchetypeRegistry registry;
		std::istringstream json(
			"{\n"
			"  \"schemaVersion\": 1,\n"
			"  \"id\": \"Broken\",\n"
			"  \"hp\": 1,\n"
			"  \"rules\": [\n"
			"    {\"effect\": \"battle.missing\"}\n"
			"  ]\n"
			"}\n");

		try
		{
			battle_sim::features::units_classic::registerDataDrivenArchetypeJson(registry, game.registries, json);
		}
		catch (const std::runtime_error& ex)
		{
			requireContains(ex.what(), "Unknown action rule handler: battle.missing");
			return;
		}

		throw std::runtime_error("Expected unknown rule handler to be rejected");
	}

	void debugSummaryUsesStderrOnly()
	{
		std::istringstream configText(
			"feature battle.basic\n"
			"feature units.classic\n"
			"feature io.legacy debugSummary=true\n");
		const auto options = battle_sim::app::runOptionsFromConfig(battle_sim::app::parseGameConfig(configText));

		const auto captured = runScenarioCaptured(
			"CREATE_MAP 1 1\n",
			options);

		requireContains(captured.output, "[1] MAP_CREATED width=1 height=1");
		requireNotContains(captured.output, "[debug]");
		requireContains(captured.error, "[debug] scenario.complete rngSeed=random archetypeFiles=0");
	}

	void jsonTraceCapturesDeterministicEvents()
	{
		std::ostringstream trace;
		battle_sim::app::RunOptions options;
		options.traceJsonOutput = &trace;

		const auto output = runScenarioText(
			"CREATE_MAP 2 1\n"
			"SPAWN_SWORDSMAN 1 0 0 5 3\n"
			"SPAWN_SWORDSMAN 2 1 0 3 1\n",
			options);

		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=0");
		requireEqual(
			trace.str(),
			"[\n"
			"  {\n"
			"    \"tick\": 1,\n"
			"    \"event\": \"MAP_CREATED\",\n"
			"    \"width\": 2,\n"
			"    \"height\": 1\n"
			"  },\n"
			"  {\n"
			"    \"tick\": 1,\n"
			"    \"event\": \"UNIT_SPAWNED\",\n"
			"    \"unitId\": 1,\n"
			"    \"unitType\": \"Swordsman\",\n"
			"    \"x\": 0,\n"
			"    \"y\": 0\n"
			"  },\n"
			"  {\n"
			"    \"tick\": 1,\n"
			"    \"event\": \"UNIT_SPAWNED\",\n"
			"    \"unitId\": 2,\n"
			"    \"unitType\": \"Swordsman\",\n"
			"    \"x\": 1,\n"
			"    \"y\": 0\n"
			"  },\n"
			"  {\n"
			"    \"tick\": 2,\n"
			"    \"event\": \"UNIT_ATTACKED\",\n"
			"    \"sourceUnitId\": 1,\n"
			"    \"targetUnitId\": 2,\n"
			"    \"sourceX\": 0,\n"
			"    \"sourceY\": 0,\n"
			"    \"targetX\": 1,\n"
			"    \"targetY\": 0,\n"
			"    \"amount\": 3,\n"
			"    \"targetHpBefore\": 3,\n"
			"    \"targetHpAfter\": 0,\n"
			"    \"resultingValue\": 0\n"
			"  },\n"
			"  {\n"
			"    \"tick\": 2,\n"
			"    \"event\": \"UNIT_DIED\",\n"
			"    \"unitId\": 2,\n"
			"    \"x\": 1,\n"
			"    \"y\": 0,\n"
			"    \"reason\": \"Death\"\n"
			"  },\n"
			"  {\n"
			"    \"tick\": 2,\n"
			"    \"event\": \"UNIT_ATTACKED\",\n"
			"    \"sourceUnitId\": 2,\n"
			"    \"targetUnitId\": 1,\n"
			"    \"sourceX\": 1,\n"
			"    \"sourceY\": 0,\n"
			"    \"targetX\": 0,\n"
			"    \"targetY\": 0,\n"
			"    \"amount\": 1,\n"
			"    \"targetHpBefore\": 5,\n"
			"    \"targetHpAfter\": 4,\n"
			"    \"resultingValue\": 4\n"
			"  }\n"
			"]\n",
			"JSON trace");
	}

	void traceInspectorPrintsSummary()
	{
		std::istringstream trace(
			"[\n"
			"  {\"tick\": 1, \"event\": \"MAP_CREATED\", \"width\": 2, \"height\": 1},\n"
			"  {\"tick\": 1, \"event\": \"UNIT_SPAWNED\", \"unitId\": 1},\n"
			"  {\"tick\": 2, \"event\": \"UNIT_MOVED\", \"unitId\": 1},\n"
			"  {\"tick\": 3, \"event\": \"UNIT_ATTACKED\", \"sourceUnitId\": 1},\n"
			"  {\"tick\": 3, \"event\": \"UNIT_DIED\", \"unitId\": 2}\n"
			"]\n");
		std::ostringstream output;

		battle_sim::app::inspectTrace(trace, output);

		requireEqual(
			output.str(),
			"Battle summary\n"
			"Ticks: 3\n"
			"Units spawned: 1\n"
			"Moves: 1\n"
			"Attacks: 1\n"
			"Deaths: 1\n",
			"trace summary");
	}

	void traceInspectorRejectsMalformedTrace()
	{
		std::istringstream trace("{\"event\":\"UNIT_SPAWNED\"}");
		std::ostringstream output;

		try
		{
			battle_sim::app::inspectTrace(trace, output);
		}
		catch (const std::runtime_error& ex)
		{
			requireContains(ex.what(), "Malformed trace: expected a JSON array");
			return;
		}

		throw std::runtime_error("Expected malformed trace to be rejected");
	}

	void genericSpawnUsesArchetypeRegistry()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 3 1\n"
			"SPAWN Swordsman 1 0 0 hp=5 strength=3\n"
			"SPAWN Swordsman 2 1 0 hp=3 strength=1\n"
			"SPAWN Mine 3 2 0 power=1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Swordsman x=0 y=0");
		requireContains(output, "[1] UNIT_SPAWNED unitId=3 unitType=Mine x=2 y=0");
		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=0");
	}

	void lancerUsesReachWithoutCoreChanges()
	{
		const auto output = runScenarioText(
			"CREATE_MAP 4 1\n"
			"SPAWN Lancer 1 0 0 hp=10 strength=4\n"
			"SPAWN Tower 2 2 0 hp=5 power=1\n");

		requireContains(output, "[1] UNIT_SPAWNED unitId=1 unitType=Lancer x=0 y=0");
		requireContains(output, "[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=4 targetHp=1");
		requireNotContains(output, "UNIT_MOVED unitId=1");
	}

	void legacyCommandsProduceCompatibilityEvents()
	{
		const auto captured = runScenarioCaptured(
			"CREATE_MAP 4 4\n"
			"SPAWN_SWORDSMAN 1 0 0 10 3\n"
			"SPAWN_HUNTER 2 3 3 10 5 2 3\n"
			"SPAWN Mine 3 1 1 power=1\n"
			"MARCH 1 2 0\n");

		requireContains(captured.output, "[1] MAP_CREATED width=4 height=4");
		requireContains(captured.output, "[1] UNIT_SPAWNED unitId=1 unitType=Swordsman x=0 y=0");
		requireContains(captured.output, "[1] UNIT_SPAWNED unitId=2 unitType=Hunter x=3 y=3");
		requireContains(captured.output, "[1] UNIT_SPAWNED unitId=3 unitType=Mine x=1 y=1");
		requireContains(captured.output, "[1] MARCH_STARTED unitId=1 x=0 y=0 targetX=2 targetY=0");
		if (!captured.error.empty())
		{
			throw std::runtime_error("Expected no validation stderr for valid legacy command scenario:\n" + captured.error);
		}
	}

	void invalidCommandsKeepValidationDiagnostics()
	{
		const auto captured = runScenarioCaptured(
			"CREATE_MAP 3 3\n"
			"SPAWN_SWORDSMAN 1 0 0 10 3\n"
			"SPAWN_HUNTER 2 2 2 10 5 2 3\n"
			"SPAWN_SWORDSMAN 1 1 1 10 1\n"
			"SPAWN_SWORDSMAN 3 0 0 10 1\n"
			"SPAWN_HUNTER 4 1 1 0 1 1 2\n"
			"SPAWN_HUNTER 5 1 1 10 -1 1 2\n"
			"SPAWN_HUNTER 6 5 5 10 1 1 2\n"
			"MARCH 0 0 0\n"
			"MARCH 999 0 0\n"
			"MARCH 2 10 10\n"
			"MARCH 2 0 0\n");

		requireContains(captured.output, "[1] MAP_CREATED width=3 height=3");
		requireContains(captured.output, "[1] UNIT_SPAWNED unitId=1 unitType=Swordsman x=0 y=0");
		requireContains(captured.output, "[1] UNIT_SPAWNED unitId=2 unitType=Hunter x=2 y=2");
		requireContains(captured.error, "validateSpawn: duplicate entityId 1");
		requireContains(captured.error, "validateSpawn: cell occupied");
		requireContains(captured.error, "validateSpawn: wrong stats");
		requireContains(captured.error, "validateSpawn: out of map");
		requireContains(captured.error, "march: entityId == 0");
		requireContains(captured.error, "march: entity not found");
		requireContains(captured.error, "march: target out of map");
	}

	void unknownCommandThrows()
	{
		try
		{
			runScenarioText(
				"CREATE_MAP 2 2\n"
				"WAIT\n");
		}
		catch (const std::runtime_error& ex)
		{
			requireContains(ex.what(), "Unknown command: WAIT");
			return;
		}

		throw std::runtime_error("Expected unknown command to throw");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"swordsmanAttacksAdjacent", swordsmanAttacksAdjacent},
		{"swordsmanMovesWhenNoAdjacentTarget", swordsmanMovesWhenNoAdjacentTarget},
		{"mobileEntityWithoutMarchDoesNotWander", mobileEntityWithoutMarchDoesNotWander},
		{"hunterShootsAtRange", hunterShootsAtRange},
		{"hunterShootsAtMaxRange", hunterShootsAtMaxRange},
		{"hunterMovesWhenTargetBeyondRange", hunterMovesWhenTargetBeyondRange},
		{"hunterUsesMeleeWhenAdjacentCellOccupied", hunterUsesMeleeWhenAdjacentCellOccupied},
		{"towerUsesPower", towerUsesPower},
		{"towerShootsAtMaxRange", towerShootsAtMaxRange},
		{"towerDoesNotMoveWithoutTarget", towerDoesNotMoveWithoutTarget},
		{"simulationStopsWhenNoEntityCanAct", simulationStopsWhenNoEntityCanAct},
		{"ravenMovesTwoCells", ravenMovesTwoCells},
		{"marchEndedReportsFinalPosition", marchEndedReportsFinalPosition},
		{"ravenAllowsBlockingEntityOnSameCell", ravenAllowsBlockingEntityOnSameCell},
		{"ravenAttacksAdjacentWithAgility", ravenAttacksAdjacentWithAgility},
		{"ravenCanBeShotAtEffectiveRange", ravenCanBeShotAtEffectiveRange},
		{"healerRestoresHealth", healerRestoresHealth},
		{"healerIgnoresNonAttackableMineAndMoves", healerIgnoresNonAttackableMineAndMoves},
		{"healerMovesWhenNoHealTarget", healerMovesWhenNoHealTarget},
		{"mineAllowsBlockingEntityOnSameCell", mineAllowsBlockingEntityOnSameCell},
		{"mineDoesNotMove", mineDoesNotMove},
		{"mineIsNotHitByTargetedAttack", mineIsNotHitByTargetedAttack},
		{"fatallyDamagedEntityFinishesCurrentTurnOnly", fatallyDamagedEntityFinishesCurrentTurnOnly},
		{"mineExplodesAndDisappears", mineExplodesAndDisappears},
		{"mineExplosionUsesPhysicalRadiusThree", mineExplosionUsesPhysicalRadiusThree},
		{"fixedSeedReplaysRandomChoices", fixedSeedReplaysRandomChoices},
		{"configSeedReplaysRandomChoices", configSeedReplaysRandomChoices},
		{"configRejectsUnknownFeature", configRejectsUnknownFeature},
		{"configRejectsDisabledRequiredFeature", configRejectsDisabledRequiredFeature},
		{"dataDrivenArchetypeLoadsFromConfig", dataDrivenArchetypeLoadsFromConfig},
		{"jsonRuleArrayDefinesSpearman", jsonRuleArrayDefinesSpearman},
		{"jsonRuleArrayRejectsUnknownHandler", jsonRuleArrayRejectsUnknownHandler},
		{"debugSummaryUsesStderrOnly", debugSummaryUsesStderrOnly},
		{"jsonTraceCapturesDeterministicEvents", jsonTraceCapturesDeterministicEvents},
		{"traceInspectorPrintsSummary", traceInspectorPrintsSummary},
		{"traceInspectorRejectsMalformedTrace", traceInspectorRejectsMalformedTrace},
		{"genericSpawnUsesArchetypeRegistry", genericSpawnUsesArchetypeRegistry},
		{"lancerUsesReachWithoutCoreChanges", lancerUsesReachWithoutCoreChanges},
		{"legacyCommandsProduceCompatibilityEvents", legacyCommandsProduceCompatibilityEvents},
		{"invalidCommandsKeepValidationDiagnostics", invalidCommandsKeepValidationDiagnostics},
		{"unknownCommandThrows", unknownCommandThrows},
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
