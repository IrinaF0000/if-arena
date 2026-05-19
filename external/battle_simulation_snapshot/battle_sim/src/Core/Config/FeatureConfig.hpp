// Generic feature configuration data model.

#pragma once

#include <string>
#include <unordered_map>

namespace battle_sim::core::config
{
	struct FeatureConfig
	{
		std::string id;
		bool enabled{true};
		std::unordered_map<std::string, std::string> flatSettings;
	};
}
