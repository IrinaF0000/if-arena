// Battle-owned policy settings used by feature systems.

#pragma once

namespace battle_sim::features::battle
{
	struct BattleOccupancyPolicy
	{
		bool singleBlockingEntityPerCell{true};
		bool nonBlockingEntitiesMayShareCells{true};
		bool enforceMapBounds{true};
	};

	struct BattleMovementPolicy
	{
		bool requireMarchTarget{true};
		bool blockingEntitiesAvoidOccupiedCells{true};
	};

	struct BattleLifecyclePolicy
	{
		bool deadEntitiesMayFinishCurrentTurn{true};
		bool cleanupDeadAfterTick{true};
		bool emitDeathBeforeCleanup{true};
	};

	struct BattleTerminationPolicy
	{
		int minimumActiveEntities{2};
		bool stopWhenNoActionsExecute{true};
	};

	struct BattlePolicies
	{
		BattleOccupancyPolicy occupancy;
		BattleMovementPolicy movement;
		BattleLifecyclePolicy lifecycle;
		BattleTerminationPolicy termination;
	};
}
