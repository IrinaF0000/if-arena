// Battle-facing scenario facade over feature-owned runtime systems.

#include "Features/Battle/BattleSimulationFacade.hpp"

#include "Features/Battle/Events/BattleEvents.hpp"
#include "Core/IWorldEventSink.hpp"
#include "Core/Engine/EngineRunner.hpp"
#include "Core/Registry/CoreHandlers.hpp"
#include "Features/Battle/BattleFeaturePack.hpp"
#include "Features/Battle/Resources/BattleMapResource.hpp"
#include "Features/Battle/Systems/BattleMarchSystem.hpp"
#include "Features/Battle/Systems/BattleSpawnSystem.hpp"
#include "Features/Battle/Systems/BattleTurnSystem.hpp"

#include <iostream>
#include <stdexcept>
#include <utility>

namespace battle_sim::features::battle
{
	BattleSimulationFacade::BattleSimulationFacade(core::IWorldEventSink& events)
		: _game(std::make_unique<core::engine::GameContext>())
	{
		_game->events.mirrorTo(&events);
		registerHandlers();
	}

	BattleSimulationFacade::BattleSimulationFacade(core::IWorldEventSink& events, std::uint32_t rngSeed)
		: _game(std::make_unique<core::engine::GameContext>(core::engine::GameSettings{rngSeed, true, 10000}))
	{
		_game->events.mirrorTo(&events);
		registerHandlers();
	}

	BattleSimulationFacade::BattleSimulationFacade(
		core::IWorldEventSink& events,
		std::unique_ptr<core::engine::GameContext> game)
		: _game(std::move(game))
	{
		if (!_game)
		{
			throw std::runtime_error("BattleSimulationFacade requires a game context");
		}
		_game->events.mirrorTo(&events);
	}

	void BattleSimulationFacade::registerHandlers()
	{
		core::registry::registerCoreHandlers(_game->registries);
		BattleFeaturePack{}.registerFeature(*_game, core::config::FeatureConfig{});
	}

	void BattleSimulationFacade::createMap(int width, int height)
	{
		if (_game->resources.contains<BattleMapResource>())
		{
			std::cerr << "[stderr] createMap: map already created\n";
			return;
		}

		_game->resources.emplace<BattleMapResource>(width, height);
		_game->events.publish(
			features::battle::MapCreatedEvent{core::Tick{1}, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)});
	}

	bool BattleSimulationFacade::spawnEntity(const EntityRecipe& recipe)
	{
		return BattleSpawnSystem::spawn(recipe, *_game, core::Tick{1});
	}

	bool BattleSimulationFacade::march(core::EntityId entityId, core::Position target)
	{
		return BattleMarchSystem::start(entityId, target, *_game, core::Tick{1});
	}

	void BattleSimulationFacade::run()
	{
		core::engine::EngineRunner runner(*_game);
		runner.run();
	}
}
