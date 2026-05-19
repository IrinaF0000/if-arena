// Neutral numeric value resolver handler contract.

#pragma once

#include "Core/Ecs/ActionRules.hpp"
#include "Core/Ecs/Entity.hpp"

namespace battle_sim::core::engine
{
	class ActionContext;
}

namespace battle_sim::core::registry
{
	class IValueResolver
	{
	public:
		virtual ~IValueResolver() = default;

		virtual int resolve(engine::ActionContext& context, ecs::EntityId actor, const ecs::ActionRule& rule) const = 0;
	};
}
