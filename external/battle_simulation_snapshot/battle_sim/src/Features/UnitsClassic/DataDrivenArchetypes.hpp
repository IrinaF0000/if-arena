// Minimal JSON-defined archetype registration for the data-driven slice.

#pragma once

#include <iosfwd>
#include <string>

namespace battle_sim::features::battle
{
	class EntityArchetypeRegistry;
}

namespace battle_sim::core::registry
{
	struct RegistryHub;
}

namespace battle_sim::features::units_classic
{
	void registerDataDrivenArchetypeJson(
		battle::EntityArchetypeRegistry& registry,
		const core::registry::RegistryHub& handlers,
		std::istream& input);
	void registerDataDrivenArchetypeFile(
		battle::EntityArchetypeRegistry& registry,
		const core::registry::RegistryHub& handlers,
		const std::string& path);
}
