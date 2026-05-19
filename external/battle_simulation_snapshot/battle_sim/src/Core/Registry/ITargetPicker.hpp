// Neutral target picking handler contract.

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
	class ITargetPicker
	{
	public:
		virtual ~ITargetPicker() = default;

		virtual std::vector<ecs::EntityId> pick(
			engine::ActionContext& context,
			ecs::EntityId actor,
			const ecs::ActionRule& rule,
			const std::vector<ecs::EntityId>& candidates) const = 0;
	};
}
