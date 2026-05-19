// Battle-owned spawn system over battle map and components.

#include "Features/Battle/Systems/BattleSpawnSystem.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Events/BattleEvents.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Policies/BattlePolicyAccess.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <iostream>

namespace battle_sim::features::battle
{
	namespace
	{
		bool validStats(const features::battle::StatComponents& stats)
		{
			return stats.health.hp > 0 && stats.strength.value >= 0 && stats.agility.value >= 0 && stats.range.value >= 0
				&& stats.spirit.value >= 0 && stats.power.value >= 0;
		}

		void applyComponents(const features::battle::EntityRecipe& recipe, core::engine::GameContext& game)
		{
			const auto id = recipe.id;
			auto& world = game.world;

			world.components<PositionComponent>().emplace(id, PositionComponent{recipe.position});
			world.components<BlocksCellComponent>().emplace(id, BlocksCellComponent{recipe.blocksCell});
			world.components<HealthComponent>().emplace(id, recipe.stats.health);
			world.components<AttackableComponent>().emplace(id, AttackableComponent{recipe.attackable});
			world.components().creationOrder.emplace(id, core::ecs::CreationOrderComponent{world.creationOrder(id)});
			world.components<StrengthComponent>().emplace(id, recipe.stats.strength);
			world.components<AgilityComponent>().emplace(id, recipe.stats.agility);
			world.components<RangeComponent>().emplace(id, recipe.stats.range);
			world.components<SpiritComponent>().emplace(id, recipe.stats.spirit);
			world.components<PowerComponent>().emplace(id, recipe.stats.power);
			world.components().actionBudget.emplace(id, recipe.actionBudget);
			world.components().actionRules.emplace(id, recipe.actionRules);
			world.components<ActionStateComponent>().emplace(id, ActionStateComponent{});

			if (recipe.mobile)
			{
				world.components<MobileComponent>().emplace(id, MobileComponent{1});
			}
			if (recipe.stats.health.hp > 0)
			{
				world.components().alive.emplace(id, core::ecs::AliveComponent{});
			}
		}
	}

	bool BattleSpawnSystem::spawn(const features::battle::EntityRecipe& recipe, core::engine::GameContext& game, core::Tick tick)
	{
		if (!game.resources.contains<BattleMapResource>())
		{
			std::cerr << "[stderr] validateSpawn: map not created\n";
			return false;
		}
		if (recipe.id == 0 || !validStats(recipe.stats))
		{
			std::cerr << "[stderr] validateSpawn: wrong stats\n";
			return false;
		}

		auto& map = game.resources.get<BattleMapResource>();
		if (!map.inside(recipe.position))
		{
			std::cerr << "[stderr] validateSpawn: out of map\n";
			return false;
		}
		if (!canPlaceAt(battlePolicies(game), map, recipe.position, recipe.blocksCell))
		{
			std::cerr << "[stderr] validateSpawn: cell occupied\n";
			return false;
		}

		if (!game.world.createEntity(recipe.id))
		{
			std::cerr << "[stderr] validateSpawn: duplicate entityId " << recipe.id << '\n';
			return false;
		}
		if (recipe.blocksCell)
		{
			map.place(recipe.id, recipe.position);
		}

		applyComponents(recipe, game);
		game.events.publish(features::battle::EntityCreatedEvent{tick, recipe.id, recipe.archetypeId, recipe.position});
		return true;
	}
}
