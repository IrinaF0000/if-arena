// This file contains Event data for UNIT_ATTACKED.

#pragma once

#include <cstdint>
#include <string>

namespace battle_sim::io
{
	struct UnitAttacked
	{
		constexpr static const char* Name = "UNIT_ATTACKED";

		uint32_t attackerUnitId{};
		uint32_t targetUnitId{};
		uint32_t damage{};
		uint32_t targetHp{};

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("attackerUnitId", attackerUnitId);
			visitor.visit("targetUnitId", targetUnitId);
			visitor.visit("damage", damage);
			visitor.visit("targetHp", targetHp);
		}
	};
}
