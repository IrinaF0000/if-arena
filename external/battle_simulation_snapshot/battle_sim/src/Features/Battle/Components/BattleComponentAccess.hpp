// Shared battle component access helpers.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/Entity.hpp"
#include "Core/Ecs/World.hpp"
#include "Features/Battle/Components/BattleComponents.hpp"

namespace battle_sim::features::battle
{
	PositionComponent* positionOf(core::ecs::World& world, core::ecs::EntityId entity);
	const PositionComponent* positionOf(const core::ecs::World& world, core::ecs::EntityId entity);

	HealthComponent* healthOf(core::ecs::World& world, core::ecs::EntityId entity);
	const HealthComponent* healthOf(const core::ecs::World& world, core::ecs::EntityId entity);

	bool isAlive(const core::ecs::World& world, core::ecs::EntityId entity);
	bool isBlocking(const core::ecs::World& world, core::ecs::EntityId entity);
	bool isAttackable(const core::ecs::World& world, core::ecs::EntityId entity);
	int distance(core::Position lhs, core::Position rhs);
}
