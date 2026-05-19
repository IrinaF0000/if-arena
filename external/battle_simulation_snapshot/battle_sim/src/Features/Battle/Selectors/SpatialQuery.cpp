// Battle-owned spatial queries over ECS battle components.

#include "Features/Battle/Selectors/SpatialQuery.hpp"

#include "Features/Battle/Components/BattleComponentAccess.hpp"

namespace battle_sim::features::battle
{
	SpatialQuery::SpatialQuery(const core::ecs::World& world)
		: _world(world)
	{
	}

	std::vector<core::ecs::EntityId> SpatialQuery::adjacentBlocking(core::ecs::EntityId actor) const
	{
		std::vector<core::ecs::EntityId> targets;
		const auto* actorPosition = positionOf(_world, actor);
		if (!actorPosition)
		{
			return targets;
		}

		for (const auto entity : _world.creationOrder())
		{
			const auto* position = positionOf(_world, entity);
			if (entity == actor || !position || !isAlive(_world, entity) || !isBlocking(_world, entity))
			{
				continue;
			}
			if (distance(actorPosition->value, position->value) == 1)
			{
				targets.push_back(entity);
			}
		}
		return targets;
	}

	std::vector<core::ecs::EntityId> SpatialQuery::effectiveRange(
		core::ecs::EntityId actor,
		int minDistance,
		int maxDistance) const
	{
		std::vector<core::ecs::EntityId> targets;
		const auto* actorPosition = positionOf(_world, actor);
		if (!actorPosition)
		{
			return targets;
		}

		for (const auto entity : _world.creationOrder())
		{
			const auto* position = positionOf(_world, entity);
			if (entity == actor || !position || !isAlive(_world, entity))
			{
				continue;
			}

			int dist = distance(actorPosition->value, position->value);
			if (!isBlocking(_world, entity) && dist > 0)
			{
				++dist;
			}
			if (dist >= minDistance && dist <= maxDistance)
			{
				targets.push_back(entity);
			}
		}
		return targets;
	}

	std::vector<core::ecs::EntityId> SpatialQuery::physicalRadius(core::ecs::EntityId actor, int maxDistance) const
	{
		std::vector<core::ecs::EntityId> targets;
		const auto* actorPosition = positionOf(_world, actor);
		if (!actorPosition)
		{
			return targets;
		}

		for (const auto entity : _world.creationOrder())
		{
			const auto* position = positionOf(_world, entity);
			if (entity == actor || !position || !isAlive(_world, entity))
			{
				continue;
			}
			if (distance(actorPosition->value, position->value) <= maxDistance)
			{
				targets.push_back(entity);
			}
		}
		return targets;
	}
}
