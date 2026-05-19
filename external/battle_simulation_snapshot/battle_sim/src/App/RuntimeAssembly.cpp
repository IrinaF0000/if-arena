// App-owned runtime composition from scenario options and feature packs.

#include "App/RuntimeAssembly.hpp"

#include "App/RegisterBuiltInFeaturePacks.hpp"
#include "Core/Registry/CoreHandlers.hpp"
#include "Core/Registry/FeaturePackCatalog.hpp"
#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/UnitsClassic/DataDrivenArchetypes.hpp"

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>

namespace battle_sim::app
{
	namespace
	{
		std::vector<core::config::FeatureConfig> defaultFeatureConfigs()
		{
			return {
				core::config::FeatureConfig{.id = "battle.basic", .enabled = true, .flatSettings = {}},
				core::config::FeatureConfig{.id = "units.classic", .enabled = true, .flatSettings = {}},
				core::config::FeatureConfig{.id = "io.legacy", .enabled = true, .flatSettings = {}}};
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

	RuntimeAssembly assembleRuntime(const RunOptions& options)
	{
		core::engine::GameSettings settings;
		settings.maxTicks = options.maxTicks;
		if (options.rngSeed.has_value())
		{
			settings.rngSeed = *options.rngSeed;
			settings.hasFixedRngSeed = true;
		}

		RuntimeAssembly assembly{std::make_unique<core::engine::GameContext>(settings)};
		auto& game = *assembly.game;
		core::registry::registerCoreHandlers(game.registries);

		core::registry::FeaturePackCatalog catalog;
		registerBuiltInFeaturePacks(catalog);

		const auto featureConfigs = options.features.empty() ? defaultFeatureConfigs() : options.features;
		std::unordered_map<std::string, bool> enabled;
		for (const auto& feature : featureConfigs)
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

		for (const auto& feature : featureConfigs)
		{
			if (!feature.enabled)
			{
				continue;
			}
			catalog.create(feature.id)->registerFeature(game, feature);
		}

		auto& archetypes = game.resources.contains<features::battle::EntityArchetypeRegistry>()
			? game.resources.get<features::battle::EntityArchetypeRegistry>()
			: game.resources.emplace<features::battle::EntityArchetypeRegistry>();
		for (const auto& archetypeFile : options.archetypeFiles)
		{
			features::units_classic::registerDataDrivenArchetypeFile(archetypes, game.registries, archetypeFile);
		}

		return assembly;
	}
}
