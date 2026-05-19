// Converts battle events into the legacy stdout event format.

#include "IO/LegacyCommands/LegacyEventAdapter.hpp"

#include "Features/Battle/Events/BattleEvents.hpp"
#include "IO/Events/MapCreated.hpp"
#include "IO/Events/MarchEnded.hpp"
#include "IO/Events/MarchStarted.hpp"
#include "IO/Events/UnitAttacked.hpp"
#include "IO/Events/UnitDied.hpp"
#include "IO/Events/UnitMoved.hpp"
#include "IO/Events/UnitSpawned.hpp"
#include "IO/LegacyCommands/EventLog.hpp"

#include <cstdint>
#include <utility>

namespace battle_sim::io
{
	namespace
	{
		std::uint32_t toUnsigned(int value)
		{
			return value <= 0 ? 0u : static_cast<std::uint32_t>(value);
		}
	}

	LegacyEventAdapter::LegacyEventAdapter(EventLog& log)
		: _log(log)
	{
	}

	void LegacyEventAdapter::registerEntityName(core::EntityId entity, std::string name)
	{
		_entityNames.insert_or_assign(entity, std::move(name));
	}

	void LegacyEventAdapter::onEvent(const std::any& event)
	{
		if (const auto* mapCreated = std::any_cast<features::battle::MapCreatedEvent>(&event))
		{
			_log.log(mapCreated->tick, MapCreated{mapCreated->width, mapCreated->height});
		}
		else if (const auto* entityCreated = std::any_cast<features::battle::EntityCreatedEvent>(&event))
		{
			_log.log(
				entityCreated->tick,
				UnitSpawned{
					entityCreated->entity,
					entityCreated->archetypeId.empty() ? entityName(entityCreated->entity) : entityCreated->archetypeId,
					toUnsigned(entityCreated->position.x),
					toUnsigned(entityCreated->position.y)});
		}
		else if (const auto* marchStarted = std::any_cast<features::battle::MarchStartedEvent>(&event))
		{
			_log.log(
				marchStarted->tick,
				MarchStarted{
					marchStarted->entity,
					toUnsigned(marchStarted->from.x),
					toUnsigned(marchStarted->from.y),
					toUnsigned(marchStarted->target.x),
					toUnsigned(marchStarted->target.y)});
		}
		else if (const auto* marchEnded = std::any_cast<features::battle::MarchEndedEvent>(&event))
		{
			_log.log(
				marchEnded->tick,
				MarchEnded{
					marchEnded->entity,
					toUnsigned(marchEnded->position.x),
					toUnsigned(marchEnded->position.y)});
		}
		else if (const auto* entityMoved = std::any_cast<features::battle::EntityMovedEvent>(&event))
		{
			_log.log(
				entityMoved->tick,
				UnitMoved{entityMoved->entity, toUnsigned(entityMoved->to.x), toUnsigned(entityMoved->to.y)});
		}
		else if (const auto* effectApplied = std::any_cast<features::battle::EffectAppliedEvent>(&event))
		{
			if (effectApplied->effect != features::battle::EffectType::Damage)
			{
				return;
			}

			_log.log(
				effectApplied->tick,
				UnitAttacked{
					effectApplied->source,
					effectApplied->target,
					toUnsigned(effectApplied->amount),
					toUnsigned(effectApplied->resultingValue)});
		}
		else if (const auto* entityRemoved = std::any_cast<features::battle::EntityRemovedEvent>(&event))
		{
			_log.log(entityRemoved->tick, UnitDied{entityRemoved->entity});
		}
	}

	const std::string& LegacyEventAdapter::entityName(core::EntityId entity) const
	{
		static const std::string UnknownEntityName = "Entity";

		const auto it = _entityNames.find(entity);
		if (it == _entityNames.end())
		{
			return UnknownEntityName;
		}
		return it->second;
	}
}
