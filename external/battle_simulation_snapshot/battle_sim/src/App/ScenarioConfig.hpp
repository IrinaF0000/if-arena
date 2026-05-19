// Scenario config loading for the CLI adapter.

#pragma once

#include "App/ScenarioRunner.hpp"
#include "Core/Config/GameConfig.hpp"

#include <iosfwd>
#include <string>

namespace battle_sim::app
{
	core::config::GameConfig defaultGameConfig();
	core::config::GameConfig parseGameConfig(std::istream& input);
	core::config::GameConfig loadGameConfig(const std::string& path);
	RunOptions runOptionsFromConfig(const core::config::GameConfig& config);
}
