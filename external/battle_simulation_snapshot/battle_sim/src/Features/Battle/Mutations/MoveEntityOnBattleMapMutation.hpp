// Battle-owned movement mutation for position, map occupancy, and events.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/Entity.hpp"
#include "Core/Engine/Mutation.hpp"

namespace battle_sim::features::battle
{
	core::engine::Mutation makeMoveEntityOnBattleMapMutation(
		core::ecs::EntityId entity,
		core::Position from,
		core::Position to,
		bool occupiesCell,
		bool reachesMarchTarget);
}
