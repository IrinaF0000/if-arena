// Neutral target selection handler contract.

#pragma once

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Ecs/Entity.hpp"

#include <vector>

namespace battle_sim::core::engine
{
	class ActionContext;
}

namespace battle_sim::core::registry
{
	class ITargetSelector
	{
	public:
		virtual ~ITargetSelector() = default;

		virtual std::vector<ecs::EntityId> select(
			engine::ActionContext& context,
			ecs::EntityId actor,
			const ecs::ActionRule& rule) const = 0;
	};
}
