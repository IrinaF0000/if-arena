// Battle-owned march command system.

#pragma once

#include "Core/CommonTypes.hpp"

namespace battle_sim::core::engine
{
	struct GameContext;
}

namespace battle_sim::features::battle
{
	class BattleMarchSystem
	{
	public:
		static bool start(core::EntityId entity, core::Position target, core::engine::GameContext& game, core::Tick tick);
	};
}
