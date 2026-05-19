// ScenarioRunner wires the application and executes a scenario stream.

#include "App/ScenarioRunner.hpp"

#include "App/RuntimeAssembly.hpp"
#include "Features/Battle/BattleSimulationFacade.hpp"
#include "Features/Battle/EntityArchetypeRegistry.hpp"
#include "IO/LegacyCommands/CommandParser.hpp"
#include "IO/LegacyCommands/EventLog.hpp"
#include "IO/LegacyCommands/LegacyEventAdapter.hpp"
#include "IO/LegacyCommands/RegisterLegacyCommands.hpp"
#include "IO/Trace/JsonTraceWriter.hpp"
#include "IO/System/TypeRegistry.hpp"

#include <any>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace battle_sim::app
{
	namespace
	{
		class CompositeWorldEventSink final : public core::IWorldEventSink
		{
		public:
			void add(core::IWorldEventSink& sink)
			{
				_sinks.push_back(&sink);
			}

			void onEvent(const std::any& event) override
			{
				for (auto* sink : _sinks)
				{
					sink->onEvent(event);
				}
			}

		private:
			std::vector<core::IWorldEventSink*> _sinks;
		};
	}

	void runScenario(std::istream& input, std::ostream& output, RunOptions options)
	{
		TypeRegistry registry;
		auto eventLog = registry.emplace<EventLog>(output);
		auto legacySink = registry.emplace<io::LegacyEventAdapter, core::IWorldEventSink>(*eventLog);
		std::unique_ptr<io::trace::JsonTraceWriter> traceWriter;
		CompositeWorldEventSink compositeSink;
		core::IWorldEventSink* eventSink = legacySink.get();
		if (options.traceJsonOutput)
		{
			traceWriter = std::make_unique<io::trace::JsonTraceWriter>(*options.traceJsonOutput);
			compositeSink.add(*legacySink);
			compositeSink.add(*traceWriter);
			eventSink = &compositeSink;
		}

		auto assembly = assembleRuntime(options);
		auto sim = registry.emplace<features::battle::BattleSimulationFacade>(*eventSink, std::move(assembly.game));
		if (!sim->gameContext().resources.contains<features::battle::EntityArchetypeRegistry>())
		{
			throw std::runtime_error("EntityArchetypeRegistry not configured");
		}
		auto& archetypes = sim->gameContext().resources.get<features::battle::EntityArchetypeRegistry>();

		io::CommandParser parser;
		io::legacy::registerLegacyCommands(parser, *sim, *legacySink, archetypes);

		parser.parse(input);
		sim->run();
		if (traceWriter)
		{
			traceWriter->finish();
		}
		if (options.debugSummary)
		{
			std::cerr << "[debug] scenario.complete rngSeed=" << (options.rngSeed.has_value() ? "fixed" : "random")
				<< " archetypeFiles=" << options.archetypeFiles.size() << '\n';
		}
	}
}
