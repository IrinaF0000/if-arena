#include "Features/UnitsClassic/ClassicUnitArchetypes.hpp"

#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/UnitsClassic/ClassicActionRuleRecipes.hpp"

namespace battle_sim::features::units_classic
{
	void registerClassicUnitArchetypes(features::battle::EntityArchetypeRegistry& registry)
	{
		registry.registerArchetype(
			"Swordsman",
			features::battle::EntityArchetype{
				.mobile = true,
				.occupiesCell = true,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeSwordsmanActionRules()});
		registry.registerArchetype(
			"Lancer",
			features::battle::EntityArchetype{
				.mobile = true,
				.occupiesCell = true,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeLancerActionRules()});
		registry.registerArchetype(
			"Hunter",
			features::battle::EntityArchetype{
				.mobile = true,
				.occupiesCell = true,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeHunterActionRules()});
		registry.registerArchetype(
			"Tower",
			features::battle::EntityArchetype{
				.mobile = false,
				.occupiesCell = true,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeTowerActionRules()});
		registry.registerArchetype(
			"Raven",
			features::battle::EntityArchetype{
				.mobile = true,
				.occupiesCell = false,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeRavenActionRules()});
		registry.registerArchetype(
			"Healer",
			features::battle::EntityArchetype{
				.mobile = true,
				.occupiesCell = true,
				.attackable = true,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeHealerActionRules()});
		registry.registerArchetype(
			"Mine",
			features::battle::EntityArchetype{
				.mobile = false,
				.occupiesCell = false,
				.attackable = false,
				.actionBudget = core::ecs::ActionBudgetComponent{.pointsPerTurn = 1},
				.actionRules = makeMineActionRules(),
				.baseStatComponents = features::battle::StatComponents{
					.health = features::battle::HealthComponent{.hp = 1},
					.strength = features::battle::StrengthComponent{},
					.agility = features::battle::AgilityComponent{},
					.range = features::battle::RangeComponent{},
					.spirit = features::battle::SpiritComponent{},
					.power = features::battle::PowerComponent{}}});
	}
}
