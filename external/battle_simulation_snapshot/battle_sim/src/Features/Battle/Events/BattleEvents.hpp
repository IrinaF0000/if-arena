// Battle-owned event DTOs emitted by battle systems and mutations.

#pragma once

#include "Core/CommonTypes.hpp"

#include <cstdint>
#include <string>

namespace battle_sim::features::battle
{
	enum class EffectType
	{
		Damage,
		Heal
	};

	enum class RemovalReason
	{
		Death,
		Destroyed
	};

	struct MapCreatedEvent
	{
		core::Tick tick{};
		std::uint32_t width{};
		std::uint32_t height{};
	};

	struct EntityCreatedEvent
	{
		core::Tick tick{};
		core::EntityId entity{};
		std::string archetypeId;
		core::Position position{};
	};

	struct MarchStartedEvent
	{
		core::Tick tick{};
		core::EntityId entity{};
		core::Position from{};
		core::Position target{};
	};

	struct MarchEndedEvent
	{
		core::Tick tick{};
		core::EntityId entity{};
		core::Position position{};
	};

	struct EntityMovedEvent
	{
		core::Tick tick{};
		core::EntityId entity{};
		core::Position from{};
		core::Position to{};
	};

	struct EffectAppliedEvent
	{
		core::Tick tick{};
		core::EntityId source{};
		core::EntityId target{};
		EffectType effect{EffectType::Damage};
		int amount{};
		int resultingValue{};
		core::Position sourcePosition{};
		core::Position targetPosition{};
		int previousValue{};
	};

	struct EntityRemovedEvent
	{
		core::Tick tick{};
		core::EntityId entity{};
		RemovalReason reason{RemovalReason::Death};
		core::Position position{};
	};
}
