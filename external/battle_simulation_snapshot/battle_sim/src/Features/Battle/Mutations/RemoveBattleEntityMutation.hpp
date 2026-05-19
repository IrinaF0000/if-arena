// Battle-owned removal mutation for world components, map state, and events.

#pragma once

#include "Core/Ecs/Entity.hpp"
#include "Core/Engine/Mutation.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"

namespace battle_sim::features::battle
{
	core::engine::Mutation makeRemoveBattleEntityMutation(
		core::ecs::EntityId entity,
		features::battle::RemovalReason reason,
		bool publishEvent);
}
