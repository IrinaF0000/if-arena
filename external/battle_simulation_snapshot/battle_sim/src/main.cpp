// Program entry point: fills TypeRegistry, parses commands, runs simulation.

#include "App/ScenarioConfig.hpp"
#include "App/ScenarioRunner.hpp"
#include "App/TraceInspector.hpp"

#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>

int main(int argc, char** argv)
{
	using namespace battle_sim;

	if (argc < 2)
	{
		throw std::runtime_error("Usage: battle_sim <commands-file> [config-file] [--trace-json <trace-file>] | battle_sim inspect <trace-file>");
	}

	if (std::string{argv[1]} == "inspect")
	{
		if (argc != 3)
		{
			throw std::runtime_error("Usage: battle_sim inspect <trace-file>");
		}
		std::ifstream traceFile(argv[2]);
		if (!traceFile)
		{
			throw std::runtime_error("Trace file not found: " + std::string(argv[2]));
		}
		app::inspectTrace(traceFile, std::cout);
		return 0;
	}

	std::optional<std::string> configPath;
	std::optional<std::string> tracePath;
	for (int arg = 2; arg < argc; ++arg)
	{
		const std::string value = argv[arg];
		if (value == "--trace-json")
		{
			if (arg + 1 >= argc)
			{
				throw std::runtime_error("Missing value for --trace-json");
			}
			tracePath = argv[++arg];
			continue;
		}
		if (configPath.has_value())
		{
			throw std::runtime_error("Usage: battle_sim <commands-file> [config-file] [--trace-json <trace-file>]");
		}
		configPath = value;
	}

	std::ifstream file(argv[1]);
	if (!file)
	{
		throw std::runtime_error("Error: File not found - " + std::string(argv[1]));
	}

	const auto config = configPath.has_value() ? app::loadGameConfig(*configPath) : app::defaultGameConfig();
	auto options = app::runOptionsFromConfig(config);
	std::ofstream traceFile;
	if (tracePath.has_value())
	{
		traceFile.open(*tracePath);
		if (!traceFile)
		{
			throw std::runtime_error("Trace file cannot be opened: " + *tracePath);
		}
		options.traceJsonOutput = &traceFile;
	}
	app::runScenario(file, std::cout, options);
	return 0;
}
