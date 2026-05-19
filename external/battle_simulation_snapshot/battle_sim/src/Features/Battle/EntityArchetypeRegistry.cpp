// Registry for battle entity archetypes.

#include "Features/Battle/EntityArchetypeRegistry.hpp"

#include <stdexcept>
#include <utility>

namespace battle_sim::features::battle
{
	void EntityArchetypeRegistry::registerArchetype(std::string id, EntityArchetype archetype)
	{
		_archetypes.insert_or_assign(std::move(id), std::move(archetype));
	}

	const EntityArchetype& EntityArchetypeRegistry::get(const std::string& id) const
	{
		const auto it = _archetypes.find(id);
		if (it == _archetypes.end())
		{
			throw std::runtime_error("Unknown battle entity archetype: " + id);
		}
		return it->second;
	}

	EntityRecipe EntityArchetypeRegistry::create(
		const std::string& id,
		core::EntityId entityId,
		core::Position pos,
		StatComponents stats) const
	{
		const auto& archetype = get(id);
		return EntityRecipe{
			.id = entityId,
			.archetypeId = id,
			.position = pos,
			.stats = stats,
			.mobile = archetype.mobile,
			.blocksCell = archetype.occupiesCell,
			.attackable = archetype.attackable,
			.actionBudget = archetype.actionBudget,
			.actionRules = archetype.actionRules};
	}
}
