// App-level JSON trace inspection.

#pragma once

#include <iosfwd>

namespace battle_sim::app
{
	void inspectTrace(std::istream& input, std::ostream& output);
}
