#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace
{
	struct LoadClientConfig
	{
		std::string endpoint{"127.0.0.1:4000"};
		std::string scenario{"connect_only"};
		std::filesystem::path output{"reports/load/dry-run-report.md"};
		std::uint32_t seed{1337};
		std::uint32_t clients{1};
		std::uint32_t durationSeconds{10};
		std::uint32_t commandRatePerSecond{1};
		bool dryRun{true};
	};

	struct LoadReport
	{
		std::uint64_t clientsStarted{};
		std::uint64_t clientsConnected{};
		std::uint64_t connectionFailures{};
		std::uint64_t commandsPlanned{};
		std::uint64_t invalidPayloadsPlanned{};
		std::uint64_t slowReadersPlanned{};
		std::uint64_t expectedRejectedCommands{};
		std::uint64_t disconnects{};
		double p50LatencyMs{};
		double p95LatencyMs{};
		double p99LatencyMs{};
	};

	constexpr std::uint32_t MaxClients = 100000;
	constexpr std::uint32_t MaxDurationSeconds = 24u * 60u * 60u;
	constexpr std::uint32_t MaxCommandRatePerSecond = 1000;
	constexpr std::uint32_t MaxSeed = 0xffffffffu;

	struct ScenarioShape
	{
		std::string description;
		std::uint32_t commandMultiplier{1};
		std::uint32_t invalidPayloadsPerClient{};
		std::uint32_t slowReaderPercent{};
		std::uint32_t rejectedCommandPercent{};
	};

	const std::map<std::string, ScenarioShape>& scenarios()
	{
		static const std::map<std::string, ScenarioShape> values{
			{"connect_only", {"open authenticated TCP sessions without gameplay commands", 0, 0, 0, 0}},
			{"create_and_join", {"pair clients into Objective Run matches and wait for snapshots", 1, 0, 0, 0}},
			{"gameplay", {"send steady movement/attack/interact intentions", 1, 0, 0, 0}},
			{"slow_readers", {"model clients that stop reading snapshots after joining", 1, 0, 25, 0}},
			{"command_spam", {"model clients sending duplicate/out-of-rate commands", 4, 0, 0, 40}},
			{"invalid_payloads", {"model malformed JSON, unknown types, and oversized frames", 0, 3, 0, 100}},
		};
		return values;
	}

	std::uint32_t parseBoundedUint(std::string_view value, std::uint32_t maxValue, std::string_view name)
	{
		std::uint64_t parsed{};
		for (const char ch : value)
		{
			if (ch < '0' || ch > '9')
			{
				throw std::runtime_error(std::string{name} + " must be an unsigned integer");
			}
			parsed = (parsed * 10u) + static_cast<std::uint64_t>(ch - '0');
			if (parsed > maxValue)
			{
				throw std::runtime_error(std::string{name} + " exceeds configured bound");
			}
		}
		if (value.empty())
		{
			throw std::runtime_error(std::string{name} + " must not be empty");
		}
		return static_cast<std::uint32_t>(parsed);
	}

	void validateScenario(std::string_view scenario)
	{
		if (scenarios().find(std::string{scenario}) == scenarios().end())
		{
			throw std::runtime_error("unknown scenario: " + std::string{scenario});
		}
	}

	std::string nextValue(int& index, int argc, char** argv, std::string_view option)
	{
		if (index + 1 >= argc)
		{
			throw std::runtime_error(std::string{"missing value for "} + std::string{option});
		}
		++index;
		return argv[index];
	}

	LoadClientConfig parseArgs(int argc, char** argv)
	{
		LoadClientConfig config;
		for (int index = 1; index < argc; ++index)
		{
			const std::string_view option{argv[index]};
			if (option == "--endpoint")
			{
				config.endpoint = nextValue(index, argc, argv, option);
			}
			else if (option == "--clients")
			{
				config.clients = parseBoundedUint(nextValue(index, argc, argv, option), MaxClients, "clients");
			}
			else if (option == "--duration")
			{
				config.durationSeconds = parseBoundedUint(nextValue(index, argc, argv, option), MaxDurationSeconds, "duration");
			}
			else if (option == "--command-rate")
			{
				config.commandRatePerSecond = parseBoundedUint(nextValue(index, argc, argv, option), MaxCommandRatePerSecond, "command-rate");
			}
			else if (option == "--scenario")
			{
				config.scenario = nextValue(index, argc, argv, option);
			}
			else if (option == "--seed")
			{
				config.seed = parseBoundedUint(nextValue(index, argc, argv, option), MaxSeed, "seed");
			}
			else if (option == "--output")
			{
				config.output = nextValue(index, argc, argv, option);
			}
			else if (option == "--dry-run")
			{
				config.dryRun = true;
			}
			else if (option == "--live")
			{
				config.dryRun = false;
			}
			else
			{
				throw std::runtime_error(std::string{"unknown option: "} + std::string{option});
			}
		}
		validateScenario(config.scenario);
		if (!config.dryRun)
		{
			throw std::runtime_error("live mode is covered by tests/load/local_tcp_load_scenarios.py for now");
		}
		return config;
	}

	LoadReport simulateDryRun(const LoadClientConfig& config)
	{
		const auto& scenario = scenarios().at(config.scenario);
		const std::uint64_t baseCommands = static_cast<std::uint64_t>(config.clients) *
		                                   static_cast<std::uint64_t>(config.durationSeconds) *
		                                   static_cast<std::uint64_t>(config.commandRatePerSecond);
		const std::uint64_t commands = baseCommands * scenario.commandMultiplier;
		const std::uint64_t slowReaders =
			(static_cast<std::uint64_t>(config.clients) * scenario.slowReaderPercent) / 100u;
		const std::uint64_t rejected = (commands * scenario.rejectedCommandPercent) / 100u;
		return LoadReport{
			config.clients,
			config.dryRun ? config.clients : 0u,
			0,
			commands,
			static_cast<std::uint64_t>(config.clients) * scenario.invalidPayloadsPerClient,
			slowReaders,
			rejected,
			0,
			0.0,
			0.0,
			0.0,
		};
	}

	void writeReport(const LoadClientConfig& config, const LoadReport& report)
	{
		if (config.output.has_parent_path())
		{
			std::filesystem::create_directories(config.output.parent_path());
		}

		std::ofstream output(config.output);
		if (!output)
		{
			throw std::runtime_error("could not open load report output");
		}

		output << "# IF Arena Load Client Report\n\n";
		output << "## Config\n\n";
		output << "- mode: " << (config.dryRun ? "dry_run" : "live") << '\n';
		output << "- endpoint: " << config.endpoint << '\n';
		output << "- scenario: " << config.scenario << '\n';
		output << "- scenarioDescription: " << scenarios().at(config.scenario).description << '\n';
		output << "- seed: " << config.seed << '\n';
		output << "- clients: " << config.clients << '\n';
		output << "- durationSeconds: " << config.durationSeconds << '\n';
		output << "- commandRatePerSecond: " << config.commandRatePerSecond << "\n\n";
		output << "## Results\n\n";
		output << "- clientsStarted: " << report.clientsStarted << '\n';
		output << "- clientsConnected: " << report.clientsConnected << '\n';
		output << "- connectionFailures: " << report.connectionFailures << '\n';
		output << "- commandsPlanned: " << report.commandsPlanned << '\n';
		output << "- invalidPayloadsPlanned: " << report.invalidPayloadsPlanned << '\n';
		output << "- slowReadersPlanned: " << report.slowReadersPlanned << '\n';
		output << "- expectedRejectedCommands: " << report.expectedRejectedCommands << '\n';
		output << "- disconnects: " << report.disconnects << '\n';
		output << "- p50LatencyMs: " << report.p50LatencyMs << '\n';
		output << "- p95LatencyMs: " << report.p95LatencyMs << '\n';
		output << "- p99LatencyMs: " << report.p99LatencyMs << "\n\n";
		output << "## Notes\n\n";
		output << "- Dry-run mode does not open sockets and must not be presented as throughput.\n";
		output << "- Live raw TCP smoke/load scenarios are in tests/load/local_tcp_load_scenarios.py.\n";
		output << "- Use this report to validate bounded scenario planning before running live local load.\n";
	}
}

int main(int argc, char** argv)
{
	try
	{
		const auto config = parseArgs(argc, argv);
		const auto report = simulateDryRun(config);
		writeReport(config, report);
		std::cout << "wrote load report: " << config.output.string() << '\n';
		return 0;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "load client error: " << ex.what() << '\n';
		return 1;
	}
}
