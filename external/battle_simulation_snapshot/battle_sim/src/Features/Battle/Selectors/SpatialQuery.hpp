// Battle-owned spatial queries over ECS battle components.

#pragma once

#include "Core/Ecs/Entity.hpp"
#include "Core/Ecs/World.hpp"

#include <vector>

namespace battle_sim::features::battle
{
	class SpatialQuery
	{
	public:
		explicit SpatialQuery(const core::ecs::World& world);

		std::vector<core::ecs::EntityId> adjacentBlocking(core::ecs::EntityId actor) const;
		std::vector<core::ecs::EntityId> effectiveRange(core::ecs::EntityId actor, int minDistance, int maxDistance) const;
		std::vector<core::ecs::EntityId> physicalRadius(core::ecs::EntityId actor, int maxDistance) const;

	private:
		const core::ecs::World& _world;
	};
}
