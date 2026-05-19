// Battle-owned data recipe for spawning an ECS entity.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/ActionRules.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"

#include <string>

namespace battle_sim::features::battle
{
	struct StatComponents
	{
		HealthComponent health;
		StrengthComponent strength;
		AgilityComponent agility;
		RangeComponent range;
		SpiritComponent spirit;
		PowerComponent power;
	};

	struct EntityRecipe
	{
		core::EntityId id{};
		std::string archetypeId;
		core::Position position{};
		StatComponents stats{};
		bool mobile{true};
		bool blocksCell{true};
		bool attackable{true};
		core::ecs::ActionBudgetComponent actionBudget{};
		core::ecs::ActionRulesComponent actionRules{};
	};
}
