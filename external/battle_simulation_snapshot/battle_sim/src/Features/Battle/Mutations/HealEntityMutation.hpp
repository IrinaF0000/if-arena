// Battle-owned heal mutation for health and events.

#pragma once

#include "Core/Ecs/Entity.hpp"
#include "Core/Engine/Mutation.hpp"

namespace battle_sim::features::battle
{
	core::engine::Mutation makeHealEntityMutation(
		core::ecs::EntityId source,
		core::ecs::EntityId target,
		int amount);
}
