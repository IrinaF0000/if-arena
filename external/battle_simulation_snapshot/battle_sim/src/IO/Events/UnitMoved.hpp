// This file contains Event data for UNIT_MOVED.

#pragma once

#include <cstdint>
#include <string>

namespace battle_sim::io
{
	struct UnitMoved
	{
		constexpr static const char* Name = "UNIT_MOVED";

		uint32_t unitId{};
		uint32_t x{};
		uint32_t y{};

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("unitId", unitId);
			visitor.visit("x", x);
			visitor.visit("y", y);
		}
	};
}
