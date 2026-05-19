// Battle-owned damage mutation for health, lifecycle marker, and events.

#pragma once

#include "Core/Ecs/Entity.hpp"
#include "Core/Engine/Mutation.hpp"

namespace battle_sim::features::battle
{
	core::engine::Mutation makeDamageEntityMutation(
		core::ecs::EntityId source,
		core::ecs::EntityId target,
		int amount);
}
