// Domain-neutral engine settings.

#pragma once

#include <cstdint>

namespace battle_sim::core::engine
{
	struct GameSettings
	{
		std::uint32_t rngSeed{0};
		bool hasFixedRngSeed{false};
		int maxTicks{10000};
	};
}
