// Registers legacy scenario commands without making feature modules depend on legacy IO.

#include "IO/LegacyCommands/RegisterLegacyCommands.hpp"

#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "Features/Battle/BattleSimulationFacade.hpp"
#include "IO/LegacyCommands/CommandParser.hpp"
#include "IO/LegacyCommands/Commands/CreateMap.hpp"
#include "IO/LegacyCommands/Commands/March.hpp"
#include "IO/LegacyCommands/Commands/Spawn.hpp"
#include "IO/LegacyCommands/Commands/SpawnHealer.hpp"
#include "IO/LegacyCommands/Commands/SpawnHunter.hpp"
#include "IO/LegacyCommands/Commands/SpawnMine.hpp"
#include "IO/LegacyCommands/Commands/SpawnRaven.hpp"
#include "IO/LegacyCommands/Commands/SpawnSwordsman.hpp"
#include "IO/LegacyCommands/Commands/SpawnTower.hpp"
#include "IO/LegacyCommands/LegacyEventAdapter.hpp"

#include <string>
#include <unordered_map>

namespace battle_sim::io::legacy
{
	namespace
	{
		void spawn(
			features::battle::BattleSimulationFacade& sim,
			LegacyEventAdapter& sink,
			const features::battle::EntityArchetypeRegistry& archetypes,
			const std::string& archetypeId,
			core::EntityId entityId,
			core::Position pos,
			features::battle::StatComponents stats)
		{
			sink.registerEntityName(entityId, archetypeId);
			sim.spawnEntity(archetypes.create(archetypeId, entityId, pos, stats));
		}

		void applyAttributes(features::battle::StatComponents& stats, const std::unordered_map<std::string, int>& attributes)
		{
			if (const auto it = attributes.find("hp"); it != attributes.end())
			{
				stats.health.hp = it->second;
			}
			if (const auto it = attributes.find("strength"); it != attributes.end())
			{
				stats.strength.value = it->second;
			}
			if (const auto it = attributes.find("agility"); it != attributes.end())
			{
				stats.agility.value = it->second;
			}
			if (const auto it = attributes.find("range"); it != attributes.end())
			{
				stats.range.value = it->second;
			}
			if (const auto it = attributes.find("spirit"); it != attributes.end())
			{
				stats.spirit.value = it->second;
			}
			if (const auto it = attributes.find("power"); it != attributes.end())
			{
				stats.power.value = it->second;
			}
		}
	}

	void registerLegacyCommands(
		CommandParser& parser,
		features::battle::BattleSimulationFacade& sim,
		LegacyEventAdapter& sink,
		const features::battle::EntityArchetypeRegistry& archetypes)
	{
		parser.add<CreateMap>(
			[&](CreateMap cmd)
			{
				sim.createMap(static_cast<int>(cmd.width), static_cast<int>(cmd.height));
			});

		parser.add<March>(
			[&](March cmd)
			{
				sim.march(
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.targetX), static_cast<int>(cmd.targetY)});
			});

		parser.add<Spawn>(
			[&](Spawn cmd)
			{
				auto stats = archetypes.get(cmd.archetypeId).baseStatComponents;
				applyAttributes(stats, cmd.attributes);
				spawn(
					sim,
					sink,
					archetypes,
					cmd.archetypeId,
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnSwordsman>(
			[&](SpawnSwordsman cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = static_cast<int>(cmd.hp);
				stats.strength.value = static_cast<int>(cmd.strength);
				spawn(
					sim,
					sink,
					archetypes,
					"Swordsman",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnHunter>(
			[&](SpawnHunter cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = static_cast<int>(cmd.hp);
				stats.agility.value = static_cast<int>(cmd.agility);
				stats.strength.value = static_cast<int>(cmd.strength);
				stats.range.value = static_cast<int>(cmd.range);
				spawn(
					sim,
					sink,
					archetypes,
					"Hunter",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnTower>(
			[&](SpawnTower cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = static_cast<int>(cmd.hp);
				stats.power.value = static_cast<int>(cmd.power);
				spawn(
					sim,
					sink,
					archetypes,
					"Tower",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnRaven>(
			[&](SpawnRaven cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = static_cast<int>(cmd.hp);
				stats.agility.value = static_cast<int>(cmd.agility);
				spawn(
					sim,
					sink,
					archetypes,
					"Raven",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnHealer>(
			[&](SpawnHealer cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = static_cast<int>(cmd.hp);
				stats.spirit.value = static_cast<int>(cmd.spirit);
				spawn(
					sim,
					sink,
					archetypes,
					"Healer",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});

		parser.add<SpawnMine>(
			[&](SpawnMine cmd)
			{
				features::battle::StatComponents stats;
				stats.health.hp = 1;
				stats.power.value = static_cast<int>(cmd.power);
				spawn(
					sim,
					sink,
					archetypes,
					"Mine",
					static_cast<core::EntityId>(cmd.unitId),
					core::Position{static_cast<int>(cmd.x), static_cast<int>(cmd.y)},
					stats);
			});
	}
}
