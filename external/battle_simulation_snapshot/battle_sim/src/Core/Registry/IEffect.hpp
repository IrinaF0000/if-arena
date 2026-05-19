// Neutral action effect handler contract.

#pragma once

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Ecs/Entity.hpp"

namespace battle_sim::core::engine
{
	class ActionContext;
}

namespace battle_sim::core::registry
{
	class IEffect
	{
	public:
		virtual ~IEffect() = default;

		virtual bool apply(
			engine::ActionContext& context,
			ecs::EntityId actor,
			ecs::EntityId target,
			const ecs::ActionRule& rule,
			int amount) const = 0;
	};
}
