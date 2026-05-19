// Shared battle component access helpers.

#include "Features/Battle/Components/BattleComponentAccess.hpp"

#include <algorithm>
#include <cstdlib>

namespace battle_sim::features::battle
{
	PositionComponent* positionOf(core::ecs::World& world, core::ecs::EntityId entity)
	{
		return world.components<PositionComponent>().get(entity);
	}

	const PositionComponent* positionOf(const core::ecs::World& world, core::ecs::EntityId entity)
	{
		return world.components<PositionComponent>().get(entity);
	}

	HealthComponent* healthOf(core::ecs::World& world, core::ecs::EntityId entity)
	{
		return world.components<HealthComponent>().get(entity);
	}

	const HealthComponent* healthOf(const core::ecs::World& world, core::ecs::EntityId entity)
	{
		return world.components<HealthComponent>().get(entity);
	}

	bool isAlive(const core::ecs::World& world, core::ecs::EntityId entity)
	{
		const auto* health = healthOf(world, entity);
		return world.exists(entity) && health && health->hp > 0;
	}

	bool isBlocking(const core::ecs::World& world, core::ecs::EntityId entity)
	{
		const auto* blocks = world.components<BlocksCellComponent>().get(entity);
		return world.exists(entity) && blocks && blocks->value;
	}

	bool isAttackable(const core::ecs::World& world, core::ecs::EntityId entity)
	{
		const auto* attackable = world.components<AttackableComponent>().get(entity);
		return world.exists(entity) && attackable && attackable->value;
	}

	int distance(core::Position lhs, core::Position rhs)
	{
		return std::max(std::abs(lhs.x - rhs.x), std::abs(lhs.y - rhs.y));
	}
}
