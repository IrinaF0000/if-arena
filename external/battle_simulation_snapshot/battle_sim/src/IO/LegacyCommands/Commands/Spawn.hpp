// Generic spawn command data.

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace battle_sim::io
{
	struct Spawn
	{
		constexpr static const char* Name = "SPAWN";

		std::string archetypeId;
		uint32_t unitId{};
		uint32_t x{};
		uint32_t y{};
		std::unordered_map<std::string, int> attributes;

		template <typename Visitor>
		void visit(Visitor& visitor)
		{
			visitor.visit("archetypeId", archetypeId);
			visitor.visit("unitId", unitId);
			visitor.visit("x", x);
			visitor.visit("y", y);

			std::string token;
			while (visitor.stream() >> token)
			{
				const auto separator = token.find('=');
				if (separator == std::string::npos || separator == 0 || separator + 1 >= token.size())
				{
					throw std::runtime_error("Invalid SPAWN attribute: " + token);
				}

				attributes[token.substr(0, separator)] = std::stoi(token.substr(separator + 1));
			}
		}
	};
}
