// Core-owned ECS components.

#pragma once

#include "Core/Ecs/ActionRules.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace battle_sim::core::ecs
{
	struct CreationOrderComponent
	{
		std::uint64_t order{};
	};

	struct AliveComponent
	{
	};

	struct RemovedComponent
	{
	};

	struct ActionStateComponent
	{
		std::unordered_map<std::string, int> counters;
	};
}
