// Command data for CREATE_MAP.

#pragma once

#include <cstdint>
#include <iosfwd>

namespace battle_sim::io
{
	struct CreateMap
	{
		constexpr static const char* Name = "CREATE_MAP";

		uint32_t width{};
		uint32_t height{};

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("width", width);
			visitor.visit("height", height);
		}
	};
}
