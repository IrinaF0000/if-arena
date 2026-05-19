#pragma once

#include "IO/LegacyCommands/details/PrintFieldVisitor.hpp"

#include <ostream>

namespace battle_sim
{
	template <typename TCommand>
	void printDebug(std::ostream& stream, TCommand& data)
	{
		stream << data.Name << ' ';
		PrintFieldVisitor visitor(stream);
		data.visit(visitor);
		stream << "\n";
	}
}
