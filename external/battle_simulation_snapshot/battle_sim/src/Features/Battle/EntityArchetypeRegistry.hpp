// Registry for battle entity archetypes.

#pragma once

#include "Core/CommonTypes.hpp"
#include "Core/Ecs/ActionRules.hpp"
#include "Features/Battle/EntityRecipe.hpp"

#include <string>
#include <unordered_map>

namespace battle_sim::features::battle
{
	struct EntityArchetype
	{
		bool mobile{true};
		bool occupiesCell{true};
		bool attackable{true};
		core::ecs::ActionBudgetComponent actionBudget;
		core::ecs::ActionRulesComponent actionRules;
		StatComponents baseStatComponents{};
	};

	class EntityArchetypeRegistry
	{
	public:
		void registerArchetype(std::string id, EntityArchetype archetype);
		const EntityArchetype& get(const std::string& id) const;
		EntityRecipe create(const std::string& id, core::EntityId entityId, core::Position pos, StatComponents stats) const;

	private:
		std::unordered_map<std::string, EntityArchetype> _archetypes;
	};
}
