// Registers the legacy text command set against the current battle scenario facade.

#pragma once

namespace battle_sim::features::battle
{
	class BattleSimulationFacade;
	class EntityArchetypeRegistry;
}

namespace battle_sim::io
{
	class CommandParser;
	class LegacyEventAdapter;
}

namespace battle_sim::io::legacy
{
	void registerLegacyCommands(
		CommandParser& parser,
		features::battle::BattleSimulationFacade& sim,
		LegacyEventAdapter& sink,
		const features::battle::EntityArchetypeRegistry& archetypes);
}
