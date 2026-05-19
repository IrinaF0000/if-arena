// Registers feature archetypes in TypeRegistry.

#include "Features/Register.hpp"

#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/UnitsClassic/ClassicUnitArchetypes.hpp"

namespace battle_sim::features
{
	void registerArchetypes(battle_sim::TypeRegistry& registry)
	{
		auto archetypes = registry.get<features::battle::EntityArchetypeRegistry>();
		if (!archetypes)
		{
			archetypes = registry.emplace<features::battle::EntityArchetypeRegistry>();
		}

		units_classic::registerClassicUnitArchetypes(*archetypes);
	}
}
