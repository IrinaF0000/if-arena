// Battle-owned ECS components.

#pragma once

#include "Core/CommonTypes.hpp"

#include <string>
#include <unordered_map>

namespace battle_sim::features::battle
{
	struct PositionComponent
	{
		core::Position value{};
	};

	struct BlocksCellComponent
	{
		bool value{true};
	};

	struct HealthComponent
	{
		int hp{};
	};

	struct AttackableComponent
	{
		bool value{true};
	};

	struct MobileComponent
	{
		int stepDistance{1};
	};

	struct MarchTargetComponent
	{
		core::Position target{};
	};

	struct StrengthComponent
	{
		int value{};
	};

	struct AgilityComponent
	{
		int value{};
	};

	struct RangeComponent
	{
		int value{};
	};

	struct SpiritComponent
	{
		int value{};
	};

	struct PowerComponent
	{
		int value{};
	};

	struct ActionStateComponent
	{
		std::unordered_map<std::string, int> counters;
	};
}
