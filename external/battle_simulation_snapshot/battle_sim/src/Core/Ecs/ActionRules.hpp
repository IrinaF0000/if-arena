// Data-only action rule DSL for ECS behavior.

#pragma once

#include "Core/Registry/HandlerId.hpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace battle_sim::core::ecs
{
	struct ActionBudgetComponent
	{
		int pointsPerTurn{1};
	};

	struct ActionRule
	{
		std::optional<registry::HandlerId> selectorHandler;
		std::optional<registry::HandlerId> targetPickerHandler;
		std::optional<registry::HandlerId> conditionHandler;
		std::optional<registry::HandlerId> effectHandler;
		std::optional<registry::HandlerId> afterEffectHandler;
		std::optional<registry::HandlerId> amountValueResolver;
		std::optional<registry::HandlerId> maxDistanceValueResolver;
		std::optional<registry::HandlerId> stepDistanceValueResolver;
		std::unordered_map<std::string, int> intParams;
		std::unordered_map<std::string, bool> boolParams;
		std::unordered_map<std::string, std::string> stringParams;
		int amount{};
		int minDistance{};
		int maxDistance{};
		int stepDistance{};
		int cost{1};
		int priority{};
		std::string stateKey;
		int stateValue{1};
	};

	struct ActionRulesComponent
	{
		std::vector<ActionRule> rules;
	};
}
