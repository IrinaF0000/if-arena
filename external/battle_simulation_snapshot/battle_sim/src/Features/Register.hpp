// Registers feature archetypes in TypeRegistry.

#pragma once

#include "IO/System/TypeRegistry.hpp"

namespace battle_sim::core
{
	class EntityArchetypeRegistry;
}

namespace battle_sim::features
{
	void registerArchetypes(battle_sim::TypeRegistry& registry);
}
