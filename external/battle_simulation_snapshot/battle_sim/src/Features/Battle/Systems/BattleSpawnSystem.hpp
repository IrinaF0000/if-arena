// Battle-owned spawn system over battle map and components.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Features/Battle/EntityRecipe.hpp"

namespace battle_sim::core::engine
{
	struct GameContext;
}

namespace battle_sim::features::battle
{
	class BattleSpawnSystem
	{
	public:
		static bool spawn(const features::battle::EntityRecipe& recipe, core::engine::GameContext& game, core::Tick tick);
	};
}
