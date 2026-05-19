// Common value types used in Core.

#pragma once

#include <cstdint>

namespace battle_sim::core
{
	// Domain identifiers are non-negative and may be logged/serialized.
	using EntityId = std::uint32_t;

	// Simulation tick used for event logging.
	using Tick = std::uint64_t;

	// Grid coordinates. Signed to make boundary checks and diffs safe.
	struct Position
	{
		int x{};
		int y{};

		bool operator==(const Position&) const = default;
	};
}
