// Battle-owned movement system over battle map and components.

#include "Features/Battle/Systems/BattleMovementSystem.hpp"

#include "Core/Engine/ActionContext.hpp"
#include "Core/Engine/Mutation.hpp"
#include "Features/Battle/Components/BattleComponentAccess.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"
#include "Features/Battle/Mutations/MoveEntityOnBattleMapMutation.hpp"
#include "Features/Battle/Policies/BattlePolicyAccess.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <algorithm>
#include <exception>
#include <random>
#include <vector>

namespace battle_sim::features::battle
{
	bool BattleMovementSystem::apply(core::engine::ActionContext& context, core::ecs::EntityId entity, int stepDistance)
	{
		auto& world = context.world();
		if (!world.exists(entity) || !world.components<MobileComponent>().has(entity))
		{
			return false;
		}
		if (!context.game().resources.contains<BattleMapResource>())
		{
			return false;
		}

		auto* position = positionOf(world, entity);
		const auto* marchTarget = world.components<MarchTargetComponent>().get(entity);
		if (!position || !marchTarget)
		{
			return false;
		}

		auto& map = context.game().resources.get<BattleMapResource>();
		const auto& policies = battlePolicies(context.game());
		const bool occupiesCell = isBlocking(world, entity);
		const core::Position from = position->value;
		core::Position current = from;
		core::Position to = from;
		const core::Position target = marchTarget->target;

		for (int step = 0; step < stepDistance; ++step)
		{
			auto candidates = map.neighbors8(current);
			std::erase_if(
				candidates,
				[&](core::Position candidate) { return !canEnterCell(policies, map, from, candidate, occupiesCell); });

			if (candidates.empty())
			{
				break;
			}

			std::vector<core::Position> preferred;
			preferred.reserve(candidates.size());
			const int currentDistance = BattleMapResource::distance(current, target);
			for (const auto candidate : candidates)
			{
				if (BattleMapResource::distance(candidate, target) < currentDistance)
				{
					preferred.push_back(candidate);
				}
			}

			auto& pool = preferred.empty() ? candidates : preferred;
			std::uniform_int_distribution<std::size_t> pick(0, pool.size() - 1);
			to = pool[pick(context.rng())];
			current = to;

			if (to == target)
			{
				break;
			}
		}

		if (to == from)
		{
			return false;
		}

		try
		{
			core::engine::MutationQueue::applyNow(
				context.game(),
				makeMoveEntityOnBattleMapMutation(entity, from, to, occupiesCell, to == target));
		}
		catch (const std::exception&)
		{
			return false;
		}

		return true;
	}
}
