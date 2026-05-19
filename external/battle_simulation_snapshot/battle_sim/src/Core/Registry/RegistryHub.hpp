// Domain-neutral aggregation point for handler registries.

#pragma once

#include "Core/Registry/HandlerRegistry.hpp"
#include "Core/Registry/ICondition.hpp"
#include "Core/Registry/IEffect.hpp"
#include "Core/Registry/ITargetPicker.hpp"
#include "Core/Registry/ITargetSelector.hpp"
#include "Core/Registry/IValueResolver.hpp"

namespace battle_sim::core::registry
{
	struct RegistryHub
	{
		HandlerRegistry<ICondition> conditions;
		HandlerRegistry<IEffect> effects;
		HandlerRegistry<ITargetSelector> targetSelectors;
		HandlerRegistry<ITargetPicker> targetPickers;
		HandlerRegistry<IValueResolver> valueResolvers;
	};
}
