// Battle-owned movement system over battle map and components.

#pragma once

#include "Core/Ecs/Entity.hpp"

namespace battle_sim::core::engine
{
	class ActionContext;
}

namespace battle_sim::features::battle
{
	class BattleMovementSystem
	{
	public:
		static bool apply(core::engine::ActionContext& context, core::ecs::EntityId entity, int stepDistance);
	};
}
