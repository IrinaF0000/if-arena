// Command data for SPAWN_HEALER.

#pragma once

#include <cstdint>

namespace battle_sim::io
{
	struct SpawnHealer
	{
		constexpr static const char* Name = "SPAWN_HEALER";

		uint32_t unitId{};
		uint32_t x{};
		uint32_t y{};
		uint32_t hp{};
		uint32_t spirit{};

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("unitId", unitId);
			visitor.visit("x", x);
			visitor.visit("y", y);
			visitor.visit("hp", hp);
			visitor.visit("spirit", spirit);
		}
	};
}
