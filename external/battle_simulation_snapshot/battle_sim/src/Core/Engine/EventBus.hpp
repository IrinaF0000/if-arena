// Generic type-erased event bus for engine-level publication.

#pragma once

#include "Core/IWorldEventSink.hpp"

#include <any>
#include <utility>
#include <vector>

namespace battle_sim::core::engine
{
	class EventBus
	{
	public:
		void mirrorTo(IWorldEventSink* sink)
		{
			_sink = sink;
		}

		template <class Event>
		void publish(Event event)
		{
			_events.emplace_back(std::move(event));
			if (_sink)
			{
				_sink->onEvent(_events.back());
			}
		}

		const std::vector<std::any>& events() const
		{
			return _events;
		}

		void clear()
		{
			_events.clear();
		}

	private:
		std::vector<std::any> _events;
		IWorldEventSink* _sink{};
	};
}
