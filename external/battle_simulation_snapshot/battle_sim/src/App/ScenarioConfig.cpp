// Scenario config loading for the CLI adapter.

#include "App/ScenarioConfig.hpp"

#include "App/RegisterBuiltInFeaturePacks.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace battle_sim::app
{
	namespace
	{
		std::string trim(std::string value)
		{
			const auto first = std::find_if_not(
				value.begin(),
				value.end(),
				[](unsigned char ch) { return std::isspace(ch) != 0; });
			const auto last = std::find_if_not(
				value.rbegin(),
				value.rend(),
				[](unsigned char ch) { return std::isspace(ch) != 0; }).base();

			if (first >= last)
			{
				return {};
			}
			return std::string(first, last);
		}

		bool parseBool(const std::string& value)
		{
			if (value == "true" || value == "1" || value == "yes")
			{
				return true;
			}
			if (value == "false" || value == "0" || value == "no")
			{
				return false;
			}
			throw std::runtime_error("Invalid boolean config value: " + value);
		}

		std::uint32_t parseUnsigned(const std::string& value, const std::string& key)
		{
			if (value.empty() || value.front() == '-')
			{
				throw std::runtime_error("Invalid numeric config value for " + key + ": " + value);
			}
			std::size_t consumed = 0;
			unsigned long parsed = 0;
			try
			{
				parsed = std::stoul(value, &consumed);
			}
			catch (const std::exception&)
			{
				throw std::runtime_error("Invalid numeric config value for " + key + ": " + value);
			}
			if (consumed != value.size())
			{
				throw std::runtime_error("Invalid numeric config value for " + key + ": " + value);
			}
			return static_cast<std::uint32_t>(parsed);
		}

		void requireFeature(const std::unordered_map<std::string, bool>& enabled, const std::string& id)
		{
			const auto it = enabled.find(id);
			if (it == enabled.end() || !it->second)
			{
				throw std::runtime_error("Required feature disabled or missing: " + id);
			}
		}
	}

	core::config::GameConfig defaultGameConfig()
	{
		core::config::GameConfig config;
		config.features.push_back(core::config::FeatureConfig{.id = "battle.basic", .enabled = true, .flatSettings = {}});
		config.features.push_back(core::config::FeatureConfig{.id = "units.classic", .enabled = true, .flatSettings = {}});
		config.features.push_back(core::config::FeatureConfig{.id = "io.legacy", .enabled = true, .flatSettings = {}});
		return config;
	}

	core::config::GameConfig parseGameConfig(std::istream& input)
	{
		core::config::GameConfig config;
		std::string line;
		int lineNumber = 0;

		while (std::getline(input, line))
		{
			++lineNumber;
			if (const auto comment = line.find('#'); comment != std::string::npos)
			{
				line = line.substr(0, comment);
			}
			line = trim(line);
			if (line.empty())
			{
				continue;
			}

			std::istringstream stream(line);
			std::string directive;
			stream >> directive;

			if (directive == "feature")
			{
				core::config::FeatureConfig feature;
				stream >> feature.id;
				if (feature.id.empty())
				{
					throw std::runtime_error("Missing feature id at config line " + std::to_string(lineNumber));
				}

				std::string token;
				while (stream >> token)
				{
					const auto separator = token.find('=');
					if (separator == std::string::npos || separator == 0 || separator + 1 >= token.size())
					{
						throw std::runtime_error("Invalid feature setting at config line " + std::to_string(lineNumber));
					}
					const auto key = token.substr(0, separator);
					const auto value = token.substr(separator + 1);
					if (key == "enabled")
					{
						feature.enabled = parseBool(value);
					}
					else
					{
						feature.flatSettings.insert_or_assign(key, value);
					}
				}

				config.features.push_back(std::move(feature));
			}
			else if (directive == "rngSeed")
			{
				std::string value;
				stream >> value;
				if (value.empty())
				{
					throw std::runtime_error("Missing rngSeed value at config line " + std::to_string(lineNumber));
				}
				if (value == "random")
				{
					config.rngSeed.reset();
				}
				else
				{
					config.rngSeed = parseUnsigned(value, "rngSeed");
				}
			}
			else if (directive == "maxTicks")
			{
				std::string value;
				stream >> value;
				if (value.empty())
				{
					throw std::runtime_error("Missing maxTicks value at config line " + std::to_string(lineNumber));
				}
				config.maxTicks = static_cast<int>(parseUnsigned(value, "maxTicks"));
			}
			else if (directive == "archetypeFile")
			{
				std::string value;
				stream >> value;
				if (value.empty())
				{
					throw std::runtime_error("Missing archetypeFile value at config line " + std::to_string(lineNumber));
				}
				config.archetypeFiles.push_back(value);
			}
			else
			{
				throw std::runtime_error("Unknown config directive at line " + std::to_string(lineNumber) + ": " + directive);
			}
		}

		return config;
	}

	core::config::GameConfig loadGameConfig(const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
		{
			throw std::runtime_error("Config file not found: " + path);
		}
		return parseGameConfig(file);
	}

	RunOptions runOptionsFromConfig(const core::config::GameConfig& config)
	{
		core::registry::FeaturePackCatalog catalog;
		registerBuiltInFeaturePacks(catalog);

		std::unordered_map<std::string, bool> enabled;
		for (const auto& feature : config.features)
		{
			if (!catalog.contains(feature.id))
			{
				throw std::runtime_error("Unknown feature pack in config: " + feature.id);
			}
			enabled.insert_or_assign(feature.id, feature.enabled);
		}

		requireFeature(enabled, "battle.basic");
		requireFeature(enabled, "units.classic");
		requireFeature(enabled, "io.legacy");

		RunOptions options;
		options.rngSeed = config.rngSeed;
		options.maxTicks = config.maxTicks;
		options.features = config.features;
		options.archetypeFiles = config.archetypeFiles;
		for (const auto& feature : config.features)
		{
			if (feature.id != "io.legacy")
			{
				continue;
			}
			if (const auto it = feature.flatSettings.find("debugSummary"); it != feature.flatSettings.end())
			{
				options.debugSummary = parseBool(it->second);
			}
		}
		return options;
	}
}
