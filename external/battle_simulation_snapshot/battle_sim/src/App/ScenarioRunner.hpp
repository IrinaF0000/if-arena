// ScenarioRunner wires the application and executes a scenario stream.

#pragma once

#include "Core/Config/FeatureConfig.hpp"

#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

namespace battle_sim::app
{
	struct RunOptions
	{
		std::optional<std::uint32_t> rngSeed;
		int maxTicks{10000};
		std::vector<core::config::FeatureConfig> features;
		std::vector<std::string> archetypeFiles;
		std::ostream* traceJsonOutput{};
		bool debugSummary{false};
	};

	void runScenario(std::istream& input, std::ostream& output, RunOptions options = {});
}
