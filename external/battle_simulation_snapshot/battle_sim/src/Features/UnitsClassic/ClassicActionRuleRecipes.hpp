// Classic unit behavior recipes expressed as ECS action rules.

#pragma once

#include "Core/Ecs/ActionRules.hpp"

namespace battle_sim::features::units_classic
{
	core::ecs::ActionRulesComponent makeSwordsmanActionRules();
	core::ecs::ActionRulesComponent makeLancerActionRules();
	core::ecs::ActionRulesComponent makeHunterActionRules();
	core::ecs::ActionRulesComponent makeTowerActionRules();
	core::ecs::ActionRulesComponent makeRavenActionRules();
	core::ecs::ActionRulesComponent makeHealerActionRules();
	core::ecs::ActionRulesComponent makeMineActionRules();
}
