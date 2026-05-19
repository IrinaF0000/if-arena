// Domain-neutral runtime context assembled from generic engine services.

#pragma once

#include "Core/Ecs/World.hpp"
#include "Core/Engine/EventBus.hpp"
#include "Core/Engine/GameSettings.hpp"
#include "Core/Engine/ResourceStore.hpp"
#include "Core/Engine/Scheduler.hpp"
#include "Core/Registry/RegistryHub.hpp"

#include <random>
#include <utility>

namespace battle_sim::core::engine
{
	struct GameContext
	{
		GameContext()
			: rng(settings.rngSeed)
		{
		}

		explicit GameContext(GameSettings initialSettings)
			: settings(std::move(initialSettings))
			, rng(settings.rngSeed)
		{
		}

		ecs::World world;
		ResourceStore resources;
		registry::RegistryHub registries;
		EventBus events;
		Scheduler scheduler;
		GameSettings settings;
		std::mt19937 rng;
	};
}
