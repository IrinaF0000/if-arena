#include "Core/Engine/Mutation.hpp"

#include "Core/Engine/GameContext.hpp"

#include <stdexcept>
#include <utility>

namespace battle_sim::core::engine
{
	namespace
	{
		void requireMutation(const Mutation& mutation)
		{
			if (!mutation)
			{
				throw std::invalid_argument("Mutation must not be empty");
			}
		}
	}

	Mutation::Mutation(Operation operation)
		: _operation(std::move(operation))
	{
	}

	Mutation::Mutation(Operation operation, AfterCommit afterCommit)
		: _operation(std::move(operation))
		, _afterCommit(std::move(afterCommit))
	{
	}

	void Mutation::apply(GameContext& game) const
	{
		requireMutation(*this);
		_operation(game);
		if (_afterCommit)
		{
			_afterCommit(game);
		}
	}

	Mutation::operator bool() const
	{
		return static_cast<bool>(_operation);
	}

	void MutationQueue::applyNow(GameContext& game, Mutation mutation)
	{
		mutation.apply(game);
	}

	void MutationQueue::applyNow(GameContext& game, Mutation::Operation operation)
	{
		applyNow(game, Mutation{std::move(operation)});
	}

	void MutationQueue::enqueue(Mutation mutation)
	{
		requireMutation(mutation);
		_mutations.push_back(std::move(mutation));
	}

	void MutationQueue::enqueue(Mutation::Operation operation)
	{
		enqueue(Mutation{std::move(operation)});
	}

	std::size_t MutationQueue::pendingCount() const
	{
		return _mutations.size();
	}

	void MutationQueue::flush(GameContext& game)
	{
		auto mutations = std::move(_mutations);
		_mutations.clear();
		for (const auto& mutation : mutations)
		{
			mutation.apply(game);
		}
	}

	void MutationQueue::clear()
	{
		_mutations.clear();
	}
}
