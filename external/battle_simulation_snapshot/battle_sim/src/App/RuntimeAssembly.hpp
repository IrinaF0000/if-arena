// App-owned runtime composition from scenario options and feature packs.

#pragma once

#include "App/ScenarioRunner.hpp"
#include "Core/Engine/GameContext.hpp"

#include <memory>

namespace battle_sim::app
{
	struct RuntimeAssembly
	{
		std::unique_ptr<core::engine::GameContext> game;
	};

	RuntimeAssembly assembleRuntime(const RunOptions& options);
}
