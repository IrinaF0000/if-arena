// Converts battle events into the legacy stdout event format.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/IWorldEventSink.hpp"

#include <any>
#include <string>
#include <unordered_map>

namespace battle_sim
{
	class EventLog;
}

namespace battle_sim::io
{
	class LegacyEventAdapter final : public core::IWorldEventSink
	{
	public:
		explicit LegacyEventAdapter(EventLog& log);

		void registerEntityName(core::EntityId entity, std::string name);
		void onEvent(const std::any& event) override;

	private:
		EventLog& _log;
		std::unordered_map<core::EntityId, std::string> _entityNames;

		const std::string& entityName(core::EntityId entity) const;
	};
}
