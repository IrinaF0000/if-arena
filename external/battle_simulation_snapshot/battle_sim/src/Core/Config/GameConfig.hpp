// Generic game configuration data model.

#pragma once

#include "Core/Config/FeatureConfig.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace battle_sim::core::config
{
	struct GameConfig
	{
		std::optional<std::uint32_t> rngSeed;
		int maxTicks{10000};
		std::vector<FeatureConfig> features;
		std::vector<std::string> archetypeFiles;
	};
}
