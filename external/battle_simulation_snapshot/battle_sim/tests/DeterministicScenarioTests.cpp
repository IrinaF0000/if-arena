#include "App/ScenarioRunner.hpp"

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

	void requireEqual(const std::string& actual, const std::string& expected, const std::string& label)
	{
		if (actual != expected)
		{
			throw std::runtime_error("Expected " + label + " to match.\nExpected:\n" + expected + "\nActual:\n" + actual);
		}
	}

	void requireContains(const std::string& text, const std::string& expected)
	{
		if (text.find(expected) == std::string::npos)
		{
			throw std::runtime_error("Expected output to contain: " + expected + "\nActual output:\n" + text);
		}
	}

	void canonicalLegacyScenarioMatchesGoldenEventLog()
	{
		battle_sim::app::RunOptions options;
		options.rngSeed = 0u;

		const auto captured = runScenarioCaptured(
			"CREATE_MAP 3 1\n"
			"SPAWN_SWORDSMAN 1 0 0 5 3\n"
			"SPAWN_SWORDSMAN 2 1 0 3 1\n",
			options);

		const std::string expectedOutput =
			"[1] MAP_CREATED width=3 height=1 \n"
			"[1] UNIT_SPAWNED unitId=1 unitType=Swordsman x=0 y=0 \n"
			"[1] UNIT_SPAWNED unitId=2 unitType=Swordsman x=1 y=0 \n"
			"[2] UNIT_ATTACKED attackerUnitId=1 targetUnitId=2 damage=3 targetHp=0 \n"
			"[2] UNIT_DIED unitId=2 \n"
			"[2] UNIT_ATTACKED attackerUnitId=2 targetUnitId=1 damage=1 targetHp=4 \n";

		requireEqual(captured.output, expectedOutput, "golden event log");
		requireEqual(captured.error, "", "validation diagnostics");
	}

	void fixedSeedScenarioRepeatsEventLog()
	{
		const std::string scenario =
			"CREATE_MAP 3 3\n"
			"SPAWN_SWORDSMAN 1 1 1 10 1\n"
			"SPAWN_SWORDSMAN 2 0 1 1 1\n"
			"SPAWN_SWORDSMAN 3 2 1 1 1\n";

		battle_sim::app::RunOptions options;
		options.rngSeed = 123u;

		const auto firstRun = runScenarioCaptured(scenario, options);
		const auto secondRun = runScenarioCaptured(scenario, options);
		const auto thirdRun = runScenarioCaptured(scenario, options);

		requireEqual(firstRun.error, "", "first stderr replay");
		requireEqual(secondRun.output, firstRun.output, "second stdout replay");
		requireEqual(thirdRun.output, firstRun.output, "third stdout replay");
		requireEqual(secondRun.error, firstRun.error, "second stderr replay");
		requireEqual(thirdRun.error, firstRun.error, "third stderr replay");
		requireContains(firstRun.output, "UNIT_ATTACKED attackerUnitId=1");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"canonicalLegacyScenarioMatchesGoldenEventLog", canonicalLegacyScenarioMatchesGoldenEventLog},
		{"fixedSeedScenarioRepeatsEventLog", fixedSeedScenarioRepeatsEventLog},
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
