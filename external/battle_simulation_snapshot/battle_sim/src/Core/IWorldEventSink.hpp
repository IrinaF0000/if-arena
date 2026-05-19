// Type-erased event sink used by the generic engine event bus.

#pragma once

#include <any>

namespace battle_sim::core
{
	class IWorldEventSink
	{
	public:
		virtual ~IWorldEventSink() = default;

		virtual void onEvent(const std::any& event) = 0;
	};
}
