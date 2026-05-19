// Neutral engine loop over scheduled phases.

#pragma once

#include "Core/Engine/GameContext.hpp"

namespace battle_sim::core::engine
{
	struct EngineRunResult
	{
		int ticksExecuted{};
		bool stopRequested{};
		bool reachedMaxTicks{};
	};

	class EngineRunner
	{
	public:
		explicit EngineRunner(GameContext& context);

		EngineRunResult run();

		void requestStop();
		void clearStopRequest();
		bool stopRequested() const;

	private:
		GameContext& _context;
		bool _stopRequested{false};
	};
}
