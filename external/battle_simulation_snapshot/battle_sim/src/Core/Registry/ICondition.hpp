// Neutral action condition handler contract.

#pragma once

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Ecs/Entity.hpp"

namespace battle_sim::core::engine
{
	class ActionContext;
}

namespace battle_sim::core::registry
{
	class ICondition
	{
	public:
		virtual ~ICondition() = default;

		virtual bool matches(engine::ActionContext& context, ecs::EntityId actor, const ecs::ActionRule& rule) const = 0;
	};
}
