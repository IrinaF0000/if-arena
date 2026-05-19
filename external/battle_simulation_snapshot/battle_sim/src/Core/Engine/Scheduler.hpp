// Deterministic generic phase scheduler.

#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace battle_sim::core::engine
{
	struct GameContext;

	enum class Phase
	{
		Startup,
		BeforeTick,
		Tick,
		AfterTick,
		Cleanup,
		Shutdown,
		Count
	};

	class IScheduledSystem
	{
	public:
		virtual ~IScheduledSystem() = default;
		virtual bool run(GameContext& context) = 0;
	};

	class Scheduler
	{
	public:
		void add(Phase phase, std::unique_ptr<IScheduledSystem> system)
		{
			if (!system)
			{
				throw std::invalid_argument("Scheduled system must not be null");
			}
			_systems[indexOf(phase)].push_back(std::move(system));
		}

		bool runPhase(Phase phase, GameContext& context)
		{
			bool stopRequested = false;
			for (const auto& system : _systems[indexOf(phase)])
			{
				stopRequested = system->run(context) || stopRequested;
			}
			return stopRequested;
		}

		bool runAllPhases(GameContext& context)
		{
			bool stopRequested = false;
			for (std::size_t phase = 0; phase < indexOf(Phase::Count); ++phase)
			{
				for (const auto& system : _systems[phase])
				{
					stopRequested = system->run(context) || stopRequested;
				}
			}
			return stopRequested;
		}

	private:
		static constexpr std::size_t indexOf(Phase phase)
		{
			return static_cast<std::size_t>(phase);
		}

		std::array<std::vector<std::unique_ptr<IScheduledSystem>>, static_cast<std::size_t>(Phase::Count)> _systems;
	};
}
