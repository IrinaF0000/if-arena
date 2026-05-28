#pragma once

#include "ArenaConfig.hpp"
#include "BattleEngine.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_backend
{
	struct PlayableScenarioConfig
	{
		std::string id{"arena_small_objective_run"};
		std::string mode{"objective_run"};
		battle_core::ArenaConfig arena;
		battle_core::ObjectiveConfig objective;
		battle_core::CombatConfig combat;
		std::uint32_t maxTicks{3600};
		double playerSpeedPerTick{0.25};
		int heroHp{100};
	};

	struct ScenarioLoadResult
	{
		std::optional<PlayableScenarioConfig> scenario;
		std::vector<std::string> errors;

		[[nodiscard]] bool ok() const
		{
			return scenario.has_value() && errors.empty();
		}
	};

	[[nodiscard]] PlayableScenarioConfig makeFallbackObjectiveRunScenario();
	[[nodiscard]] ScenarioLoadResult parseScenarioConfig(std::string_view json);
}
