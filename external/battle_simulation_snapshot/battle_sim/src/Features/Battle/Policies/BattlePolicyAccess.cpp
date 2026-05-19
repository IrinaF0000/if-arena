// Accessors and decisions for Battle-owned policies.

#include "Features/Battle/Policies/BattlePolicyAccess.hpp"

#include "Core/Engine/GameContext.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

namespace battle_sim::features::battle
{
	BattlePolicies& battlePolicies(core::engine::GameContext& game)
	{
		if (!game.resources.contains<BattlePolicies>())
		{
			game.resources.emplace<BattlePolicies>();
		}
		return game.resources.get<BattlePolicies>();
	}

	const BattlePolicies& battlePolicies(const core::engine::GameContext& game)
	{
		return game.resources.get<BattlePolicies>();
	}

	bool canPlaceAt(const BattlePolicies& policies, const BattleMapResource& map, core::Position position, bool blocksCell)
	{
		if (policies.occupancy.enforceMapBounds && !map.inside(position))
		{
			return false;
		}
		if (blocksCell && policies.occupancy.singleBlockingEntityPerCell && map.isOccupied(position))
		{
			return false;
		}
		return true;
	}

	bool canEnterCell(
		const BattlePolicies& policies,
		const BattleMapResource& map,
		core::Position from,
		core::Position candidate,
		bool occupiesCell)
	{
		if (policies.occupancy.enforceMapBounds && !map.inside(candidate))
		{
			return false;
		}
		if (occupiesCell && policies.movement.blockingEntitiesAvoidOccupiedCells && candidate != from && map.isOccupied(candidate))
		{
			return false;
		}
		return true;
	}

	bool canActThisTurn(const BattlePolicies& policies, const core::ecs::World& world, core::ecs::EntityId entity)
	{
		if (!world.exists(entity))
		{
			return false;
		}
		return policies.lifecycle.deadEntitiesMayFinishCurrentTurn || isAlive(world, entity);
	}

	bool shouldCleanupDeadAfterTick(const BattlePolicies& policies)
	{
		return policies.lifecycle.cleanupDeadAfterTick;
	}

	bool hasEnoughActiveEntities(const BattlePolicies& policies, const core::ecs::World& world)
	{
		return static_cast<int>(world.activeCount()) >= policies.termination.minimumActiveEntities;
	}

	bool shouldStopAfterNoActions(const BattlePolicies& policies, int actionsExecuted)
	{
		return policies.termination.stopWhenNoActionsExecute && actionsExecuted == 0;
	}
}
