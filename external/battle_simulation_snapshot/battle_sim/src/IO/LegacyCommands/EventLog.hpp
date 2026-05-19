// EventLog: prints events to stdout in required format.

#pragma once

#include "IO/LegacyCommands/details/PrintFieldVisitor.hpp"
#include <iostream>
#include <typeindex>
#include <unordered_map>

namespace battle_sim
{
	class EventLog
	{
	public:
		explicit EventLog(std::ostream& stream = std::cout)
			: _stream(stream)
		{
		}

		template <class TEvent>
		void log(uint64_t tick, TEvent&& event)
		{
			_stream << "[" << tick << "] " << TEvent::Name << " ";
			PrintFieldVisitor visitor(_stream);
			event.visit(visitor);
			_stream << std::endl;
		}

	private:
		std::ostream& _stream;
	};
}
