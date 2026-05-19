// Neutral engine loop over scheduled phases.

#include "Core/Engine/EngineRunner.hpp"

#include <algorithm>

namespace battle_sim::core::engine
{
	EngineRunner::EngineRunner(GameContext& context)
		: _context(context)
	{
	}

	EngineRunResult EngineRunner::run()
	{
		int ticksExecuted = 0;
		const int maxTicks = std::max(0, _context.settings.maxTicks);

		if (_context.scheduler.runPhase(Phase::Startup, _context))
		{
			requestStop();
		}

		while (!_stopRequested && ticksExecuted < maxTicks)
		{
			if (_context.scheduler.runPhase(Phase::BeforeTick, _context))
			{
				requestStop();
			}
			if (_stopRequested)
			{
				break;
			}

			if (_context.scheduler.runPhase(Phase::Tick, _context))
			{
				requestStop();
			}
			if (_stopRequested)
			{
				break;
			}

			if (_context.scheduler.runPhase(Phase::AfterTick, _context))
			{
				requestStop();
			}
			if (_stopRequested)
			{
				break;
			}

			if (_context.scheduler.runPhase(Phase::Cleanup, _context))
			{
				requestStop();
			}
			if (_stopRequested)
			{
				break;
			}
			++ticksExecuted;
			_context.world.advanceTick();
		}

		_context.scheduler.runPhase(Phase::Shutdown, _context);

		return EngineRunResult{
			.ticksExecuted = ticksExecuted,
			.stopRequested = _stopRequested,
			.reachedMaxTicks = !_stopRequested && ticksExecuted >= maxTicks};
	}

	void EngineRunner::requestStop()
	{
		_stopRequested = true;
	}

	void EngineRunner::clearStopRequest()
	{
		_stopRequested = false;
	}

	bool EngineRunner::stopRequested() const
	{
		return _stopRequested;
	}
}
