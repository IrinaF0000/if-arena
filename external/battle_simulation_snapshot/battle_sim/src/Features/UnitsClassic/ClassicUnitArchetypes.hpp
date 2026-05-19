// Classic unit archetype registration.

#pragma once

namespace battle_sim::features::battle
{
	class EntityArchetypeRegistry;
}

namespace battle_sim::features::units_classic
{
	void registerClassicUnitArchetypes(battle::EntityArchetypeRegistry& registry);
}
