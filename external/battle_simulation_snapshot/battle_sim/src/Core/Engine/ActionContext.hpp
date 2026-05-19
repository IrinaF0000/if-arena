// Domain-neutral action handler context.

#pragma once

#include "Core/Ecs/World.hpp"
#include "Core/Engine/EventBus.hpp"
#include "Core/Engine/GameContext.hpp"
#include "Core/Engine/Mutation.hpp"
#include "Core/Registry/RegistryHub.hpp"

#include <cstddef>
#include <random>
#include <utility>

namespace battle_sim::core::engine
{
	class ActionContext
	{
	public:
		using Mutation = MutationQueue::Mutation;

		explicit ActionContext(GameContext& game)
			: _game(game)
		{
		}

		GameContext& game()
		{
			return _game;
		}

		ecs::World& world()
		{
			return _game.world;
		}

		registry::RegistryHub& registries()
		{
			return _game.registries;
		}

		EventBus& events()
		{
			return _game.events;
		}

		std::mt19937& rng()
		{
			return _game.rng;
		}

		void enqueueMutation(Mutation mutation)
		{
			_mutations.enqueue(std::move(mutation));
		}

		void enqueueMutation(Mutation::Operation operation)
		{
			_mutations.enqueue(std::move(operation));
		}

		std::size_t pendingMutationCount() const
		{
			return _mutations.pendingCount();
		}

		void flushMutations()
		{
			_mutations.flush(_game);
		}

		void clearMutations()
		{
			_mutations.clear();
		}

	private:
		GameContext& _game;
		MutationQueue _mutations;
	};
}
