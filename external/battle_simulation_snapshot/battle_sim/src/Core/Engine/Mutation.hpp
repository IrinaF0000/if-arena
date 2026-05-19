// Domain-neutral state mutation API.

#pragma once

#include <cstddef>
#include <functional>
#include <vector>

namespace battle_sim::core::engine
{
	struct GameContext;

	class Mutation
	{
	public:
		using Operation = std::function<void(GameContext&)>;
		using AfterCommit = std::function<void(GameContext&)>;

		Mutation() = default;
		Mutation(Operation operation);
		Mutation(Operation operation, AfterCommit afterCommit);

		void apply(GameContext& game) const;

		explicit operator bool() const;

	private:
		Operation _operation;
		AfterCommit _afterCommit;
	};

	class MutationQueue
	{
	public:
		using Mutation = battle_sim::core::engine::Mutation;

		static void applyNow(GameContext& game, Mutation mutation);
		static void applyNow(GameContext& game, Mutation::Operation operation);

		void enqueue(Mutation mutation);
		void enqueue(Mutation::Operation operation);
		std::size_t pendingCount() const;
		void flush(GameContext& game);
		void clear();

	private:
		std::vector<Mutation> _mutations;
	};
}
