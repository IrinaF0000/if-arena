// Minimal JSON-defined archetype registration for the M7 data slice.

#include "Features/UnitsClassic/DataDrivenArchetypes.hpp"

#include "Core/Registry/RegistryHub.hpp"
#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/Battle/ActionRuleParams.hpp"
#include "Features/UnitsClassic/ClassicActionRuleRecipes.hpp"

#include <cstddef>
#include <fstream>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace battle_sim::features::units_classic
{
	namespace
	{
		std::string readAll(std::istream& input)
		{
			std::ostringstream buffer;
			buffer << input.rdbuf();
			return buffer.str();
		}

		std::optional<std::string> stringField(const std::string& json, const std::string& key)
		{
			const std::regex pattern("\"" + key + "\"\\s*:\\s*\"([^\"]*)\"");
			std::smatch match;
			if (!std::regex_search(json, match, pattern))
			{
				return std::nullopt;
			}
			return match[1].str();
		}

		std::optional<int> intField(const std::string& json, const std::string& key)
		{
			const std::regex pattern("\"" + key + "\"\\s*:\\s*(-?[0-9]+)");
			std::smatch match;
			if (!std::regex_search(json, match, pattern))
			{
				return std::nullopt;
			}
			return std::stoi(match[1].str());
		}

		std::optional<bool> boolField(const std::string& json, const std::string& key)
		{
			const std::regex pattern("\"" + key + "\"\\s*:\\s*(true|false)");
			std::smatch match;
			if (!std::regex_search(json, match, pattern))
			{
				return std::nullopt;
			}
			return match[1].str() == "true";
		}

		bool hasField(const std::string& json, const std::string& key)
		{
			const std::regex pattern("\"" + key + "\"\\s*:");
			return std::regex_search(json, pattern);
		}

		std::string arrayBody(const std::string& json, const std::string& key)
		{
			const std::regex pattern("\"" + key + "\"\\s*:\\s*\\[");
			std::smatch match;
			if (!std::regex_search(json, match, pattern))
			{
				return {};
			}

			const std::size_t begin = static_cast<std::size_t>(match.position()) + static_cast<std::size_t>(match.length());
			int depth = 1;
			bool inString = false;
			bool escaped = false;
			for (std::size_t index = begin; index < json.size(); ++index)
			{
				const char ch = json[index];
				if (escaped)
				{
					escaped = false;
					continue;
				}
				if (ch == '\\')
				{
					escaped = inString;
					continue;
				}
				if (ch == '"')
				{
					inString = !inString;
					continue;
				}
				if (inString)
				{
					continue;
				}
				if (ch == '[')
				{
					++depth;
				}
				else if (ch == ']')
				{
					--depth;
					if (depth == 0)
					{
						return json.substr(begin, index - begin);
					}
				}
			}
			throw std::runtime_error("Invalid rule array");
		}

		std::vector<std::string> objectBodies(const std::string& array)
		{
			std::vector<std::string> objects;
			bool inString = false;
			bool escaped = false;
			int depth = 0;
			std::size_t begin = 0;
			for (std::size_t index = 0; index < array.size(); ++index)
			{
				const char ch = array[index];
				if (escaped)
				{
					escaped = false;
					continue;
				}
				if (ch == '\\')
				{
					escaped = inString;
					continue;
				}
				if (ch == '"')
				{
					inString = !inString;
					continue;
				}
				if (inString)
				{
					continue;
				}
				if (ch == '{')
				{
					if (depth == 0)
					{
						begin = index;
					}
					++depth;
				}
				else if (ch == '}')
				{
					--depth;
					if (depth < 0)
					{
						throw std::runtime_error("Invalid rule object");
					}
					if (depth == 0)
					{
						objects.push_back(array.substr(begin, index - begin + 1));
					}
				}
			}
			if (depth != 0 || inString)
			{
				throw std::runtime_error("Invalid rule object");
			}
			return objects;
		}

		void requireHandler(bool exists, const std::string& id)
		{
			if (!exists)
			{
				throw std::runtime_error("Unknown action rule handler: " + id);
			}
		}

		core::registry::HandlerId handlerId(const std::string& id)
		{
			return core::registry::HandlerId::fromString(id);
		}

		core::ecs::ActionRulesComponent rulesFromJson(
			const std::string& json,
			const core::registry::RegistryHub& handlers)
		{
			const auto body = arrayBody(json, "rules");
			const auto objects = objectBodies(body);
			if (objects.empty())
			{
				throw std::runtime_error("Data-driven archetype requires non-empty rules");
			}

			core::ecs::ActionRulesComponent rules;
			for (const auto& object : objects)
			{
				const auto effect = stringField(object, "effect");
				if (!effect.has_value() || effect->empty())
				{
					throw std::runtime_error("Invalid rule object: missing effect");
				}
				requireHandler(handlers.effects.contains(*effect), *effect);

				core::ecs::ActionRule rule;
				rule.effectHandler = handlerId(*effect);
				rule.priority = intField(object, "priority").value_or(0);
				rule.cost = intField(object, "cost").value_or(1);
				if (rule.cost <= 0)
				{
					throw std::runtime_error("Invalid rule object: cost must be positive");
				}
				rule.amount = intField(object, "amount").value_or(0);
				rule.minDistance = intField(object, "minDistance").value_or(0);
				rule.maxDistance = intField(object, "maxDistance").value_or(0);
				rule.stepDistance = intField(object, "stepDistance").value_or(0);

				if (const auto condition = stringField(object, "condition"))
				{
					requireHandler(handlers.conditions.contains(*condition), *condition);
					rule.conditionHandler = handlerId(*condition);
				}
				if (const auto selector = stringField(object, "selector"))
				{
					requireHandler(handlers.targetSelectors.contains(*selector), *selector);
					rule.selectorHandler = handlerId(*selector);
				}
				if (const auto targetPicker = stringField(object, "targetPicker"))
				{
					requireHandler(handlers.targetPickers.contains(*targetPicker), *targetPicker);
					rule.targetPickerHandler = handlerId(*targetPicker);
				}
				if (const auto amountResolver = stringField(object, "amountValueResolver"))
				{
					requireHandler(handlers.valueResolvers.contains(*amountResolver), *amountResolver);
					rule.amountValueResolver = handlerId(*amountResolver);
				}
				if (const auto maxDistanceResolver = stringField(object, "maxDistanceValueResolver"))
				{
					requireHandler(handlers.valueResolvers.contains(*maxDistanceResolver), *maxDistanceResolver);
					rule.maxDistanceValueResolver = handlerId(*maxDistanceResolver);
				}
				if (const auto stepDistanceResolver = stringField(object, "stepDistanceValueResolver"))
				{
					requireHandler(handlers.valueResolvers.contains(*stepDistanceResolver), *stepDistanceResolver);
					rule.stepDistanceValueResolver = handlerId(*stepDistanceResolver);
				}
				if (const auto allowEmptyTargets = boolField(object, "allowEmptyTargets"))
				{
					rule.boolParams["allow-empty-targets"] = *allowEmptyTargets;
				}
				if (const auto requireAttackableTarget = boolField(object, "requireAttackableTarget"))
				{
					rule.boolParams[std::string{battle::RequireAttackableTargetParam}] = *requireAttackableTarget;
				}

				rules.rules.push_back(std::move(rule));
			}
			return rules;
		}

		core::ecs::ActionRulesComponent rulesForTemplate(const std::string& actionTemplate)
		{
			if (actionTemplate == "adjacent-striker")
			{
				return makeSwordsmanActionRules();
			}
			if (actionTemplate == "lancer-reach")
			{
				return makeLancerActionRules();
			}
			throw std::runtime_error("Unknown data-driven actionTemplate: " + actionTemplate);
		}
	}

	void registerDataDrivenArchetypeJson(
		features::battle::EntityArchetypeRegistry& registry,
		const core::registry::RegistryHub& handlers,
		std::istream& input)
	{
		const auto json = readAll(input);
		if (const auto schemaVersion = intField(json, "schemaVersion"))
		{
			if (*schemaVersion != 1)
			{
				throw std::runtime_error("Unsupported data-driven archetype schemaVersion");
			}
		}

		const auto id = stringField(json, "id");
		if (!id.has_value() || id->empty())
		{
			throw std::runtime_error("Data-driven archetype requires string field: id");
		}

		const auto actionTemplate = stringField(json, "actionTemplate").value_or("adjacent-striker");
		const auto hp = intField(json, "hp");
		if (!hp.has_value() || *hp <= 0)
		{
			throw std::runtime_error("Data-driven archetype requires positive integer field: hp");
		}

		features::battle::StatComponents stats;
		stats.health.hp = *hp;
		stats.strength.value = intField(json, "strength").value_or(0);
		stats.agility.value = intField(json, "agility").value_or(0);
		stats.range.value = intField(json, "range").value_or(0);
		stats.spirit.value = intField(json, "spirit").value_or(0);
		stats.power.value = intField(json, "power").value_or(0);

		features::battle::EntityArchetype archetype;
		archetype.mobile = boolField(json, "mobile").value_or(true);
		archetype.occupiesCell = boolField(json, "blocksCell").value_or(true);
		archetype.attackable = boolField(json, "attackable").value_or(true);
		archetype.actionBudget.pointsPerTurn = intField(json, "actionBudget").value_or(1);
		if (archetype.actionBudget.pointsPerTurn <= 0)
		{
			throw std::runtime_error("Data-driven archetype requires positive actionBudget");
		}
		archetype.actionRules = hasField(json, "rules") ? rulesFromJson(json, handlers) : rulesForTemplate(actionTemplate);
		archetype.baseStatComponents = stats;

		registry.registerArchetype(*id, std::move(archetype));
	}

	void registerDataDrivenArchetypeFile(
		features::battle::EntityArchetypeRegistry& registry,
		const core::registry::RegistryHub& handlers,
		const std::string& path)
	{
		std::ifstream file(path);
		if (!file)
		{
			throw std::runtime_error("Data-driven archetype file not found: " + path);
		}
		registerDataDrivenArchetypeJson(registry, handlers, file);
	}
}
