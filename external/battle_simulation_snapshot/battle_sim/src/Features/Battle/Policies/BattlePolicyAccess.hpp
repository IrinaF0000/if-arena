// Accessors and decisions for Battle-owned policies.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/World.hpp"
#include "Features/Battle/Policies/BattlePolicies.hpp"

namespace battle_sim::core::engine
{
	struct GameContext;
}

namespace battle_sim::features::battle
{
	class BattleMapResource;

	BattlePolicies& battlePolicies(core::engine::GameContext& game);
	const BattlePolicies& battlePolicies(const core::engine::GameContext& game);

	bool canPlaceAt(const BattlePolicies& policies, const BattleMapResource& map, core::Position position, bool blocksCell);
	bool canEnterCell(
		const BattlePolicies& policies,
		const BattleMapResource& map,
		core::Position from,
		core::Position candidate,
		bool occupiesCell);
	bool canActThisTurn(const BattlePolicies& policies, const core::ecs::World& world, core::ecs::EntityId entity);
	bool shouldCleanupDeadAfterTick(const BattlePolicies& policies);
	bool hasEnoughActiveEntities(const BattlePolicies& policies, const core::ecs::World& world);
	bool shouldStopAfterNoActions(const BattlePolicies& policies, int actionsExecuted);
}
