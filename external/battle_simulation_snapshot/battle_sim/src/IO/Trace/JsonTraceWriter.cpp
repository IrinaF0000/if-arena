// Deterministic JSON trace writer for world events.

#include "IO/Trace/JsonTraceWriter.hpp"

#include "Features/Battle/Events/BattleEvents.hpp"

#include <any>
#include <ostream>
#include <string_view>

namespace battle_sim::io::trace
{
	namespace
	{
		void writeString(std::ostream& output, std::string_view value)
		{
			output << '"';
			for (const char ch : value)
			{
				switch (ch)
				{
				case '\\':
					output << "\\\\";
					break;
				case '"':
					output << "\\\"";
					break;
				case '\n':
					output << "\\n";
					break;
				case '\r':
					output << "\\r";
					break;
				case '\t':
					output << "\\t";
					break;
				default:
					output << ch;
					break;
				}
			}
			output << '"';
		}

		void writeHeader(std::ostream& output, core::Tick tick, std::string_view eventName)
		{
			output << "    \"tick\": " << tick << ",\n";
			output << "    \"event\": ";
			writeString(output, eventName);
		}

		std::string_view removalReasonName(features::battle::RemovalReason reason)
		{
			switch (reason)
			{
			case features::battle::RemovalReason::Death:
				return "Death";
			case features::battle::RemovalReason::Destroyed:
				return "Destroyed";
			}
			return "Unknown";
		}
	}

	JsonTraceWriter::JsonTraceWriter(std::ostream& output)
		: _output(output)
	{
		_output << "[\n";
	}

	JsonTraceWriter::~JsonTraceWriter()
	{
		finish();
	}

	void JsonTraceWriter::beginRecord()
	{
		if (_hasRecords)
		{
			_output << ",\n";
		}
		_output << "  {\n";
		_hasRecords = true;
	}

	void JsonTraceWriter::endRecord()
	{
		_output << "\n  }";
	}

	void JsonTraceWriter::onEvent(const std::any& event)
	{
		using namespace features::battle;

		if (const auto* mapCreated = std::any_cast<MapCreatedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, mapCreated->tick, "MAP_CREATED");
			_output << ",\n    \"width\": " << mapCreated->width << ",\n";
			_output << "    \"height\": " << mapCreated->height;
			endRecord();
		}
		else if (const auto* entityCreated = std::any_cast<EntityCreatedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, entityCreated->tick, "UNIT_SPAWNED");
			_output << ",\n    \"unitId\": " << entityCreated->entity << ",\n";
			_output << "    \"unitType\": ";
			writeString(_output, entityCreated->archetypeId);
			_output << ",\n    \"x\": " << entityCreated->position.x << ",\n";
			_output << "    \"y\": " << entityCreated->position.y;
			endRecord();
		}
		else if (const auto* marchStarted = std::any_cast<MarchStartedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, marchStarted->tick, "MARCH_STARTED");
			_output << ",\n    \"unitId\": " << marchStarted->entity << ",\n";
			_output << "    \"x\": " << marchStarted->from.x << ",\n";
			_output << "    \"y\": " << marchStarted->from.y << ",\n";
			_output << "    \"targetX\": " << marchStarted->target.x << ",\n";
			_output << "    \"targetY\": " << marchStarted->target.y;
			endRecord();
		}
		else if (const auto* marchEnded = std::any_cast<MarchEndedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, marchEnded->tick, "MARCH_ENDED");
			_output << ",\n    \"unitId\": " << marchEnded->entity << ",\n";
			_output << "    \"x\": " << marchEnded->position.x << ",\n";
			_output << "    \"y\": " << marchEnded->position.y;
			endRecord();
		}
		else if (const auto* entityMoved = std::any_cast<EntityMovedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, entityMoved->tick, "UNIT_MOVED");
			_output << ",\n    \"unitId\": " << entityMoved->entity << ",\n";
			_output << "    \"fromX\": " << entityMoved->from.x << ",\n";
			_output << "    \"fromY\": " << entityMoved->from.y << ",\n";
			_output << "    \"x\": " << entityMoved->to.x << ",\n";
			_output << "    \"y\": " << entityMoved->to.y;
			endRecord();
		}
		else if (const auto* effectApplied = std::any_cast<EffectAppliedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, effectApplied->tick, effectApplied->effect == EffectType::Damage ? "UNIT_ATTACKED" : "UNIT_HEALED");
			_output << ",\n    \"sourceUnitId\": " << effectApplied->source << ",\n";
			_output << "    \"targetUnitId\": " << effectApplied->target << ",\n";
			_output << "    \"sourceX\": " << effectApplied->sourcePosition.x << ",\n";
			_output << "    \"sourceY\": " << effectApplied->sourcePosition.y << ",\n";
			_output << "    \"targetX\": " << effectApplied->targetPosition.x << ",\n";
			_output << "    \"targetY\": " << effectApplied->targetPosition.y << ",\n";
			_output << "    \"amount\": " << effectApplied->amount << ",\n";
			_output << "    \"targetHpBefore\": " << effectApplied->previousValue << ",\n";
			_output << "    \"targetHpAfter\": " << effectApplied->resultingValue << ",\n";
			_output << "    \"resultingValue\": " << effectApplied->resultingValue;
			endRecord();
		}
		else if (const auto* entityRemoved = std::any_cast<EntityRemovedEvent>(&event))
		{
			beginRecord();
			writeHeader(_output, entityRemoved->tick, "UNIT_DIED");
			_output << ",\n    \"unitId\": " << entityRemoved->entity << ",\n";
			_output << "    \"x\": " << entityRemoved->position.x << ",\n";
			_output << "    \"y\": " << entityRemoved->position.y << ",\n";
			_output << "    \"reason\": ";
			writeString(_output, removalReasonName(entityRemoved->reason));
			endRecord();
		}
	}

	void JsonTraceWriter::finish()
	{
		if (_finished)
		{
			return;
		}
		_output << "\n]\n";
		_finished = true;
	}
}
