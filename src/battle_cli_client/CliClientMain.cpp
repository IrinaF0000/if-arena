#include "Protocol.hpp"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	using if_arena::battle_protocol::ClientSessionPhase;
	using if_arena::battle_protocol::Envelope;
	using if_arena::battle_protocol::MessageType;
	using if_arena::battle_protocol::ProtocolErrorCode;
	using if_arena::battle_protocol::serializeEnvelope;
	using if_arena::battle_protocol::validateClientEnvelope;

	struct CliOptions
	{
		std::string endpoint{"127.0.0.1:5555"};
		std::string authMode{"demo"};
		std::string displayName{"cli-player"};
		std::string matchId{"local-match"};
		std::string joinCode{"LOCAL1"};
		std::string scriptPath;
		bool fakeConnect{true};
		bool createMatch{true};
		bool interactive{};
		bool help{};
	};

	struct ScriptCommand
	{
		std::string kind;
		int dx{};
		int dy{};
		bool hasDirection{};
	};

	std::string jsonString(std::string_view value);

	void printHelp()
	{
		std::cout << "Usage: battle_cli_client [--fake-connect] [--endpoint HOST:PORT] [--auth-mode demo]\n"
		          << "                         [--create | --join CODE] [--match-id ID]\n"
		          << "                         [--script PATH] [--interactive]\n"
		          << "\n"
		          << "Current task scope: fake-connect mode builds validated protocol intentions and prints a readable\n"
		          << "transcript. Real TCP connection is intentionally deferred to the TCP vertical slice.\n";
	}

	void addError(std::vector<std::string>& errors, std::string message)
	{
		errors.push_back(std::move(message));
	}

	CliOptions parseArgs(int argc, char** argv, std::vector<std::string>& errors)
	{
		CliOptions options;
		for (int index = 1; index < argc; ++index)
		{
			const std::string_view arg{argv[index]};
			const auto requireValue = [&](std::string_view name) -> std::optional<std::string> {
				if (index + 1 >= argc)
				{
					addError(errors, std::string{name} + " requires a value");
					return std::nullopt;
				}
				return std::string{argv[++index]};
			};

			if (arg == "--help" || arg == "-h")
			{
				options.help = true;
			}
			else if (arg == "--fake-connect")
			{
				options.fakeConnect = true;
			}
			else if (arg == "--endpoint")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.endpoint = *value;
				}
			}
			else if (arg == "--auth-mode")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.authMode = *value;
				}
			}
			else if (arg == "--display-name")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.displayName = *value;
				}
			}
			else if (arg == "--create")
			{
				options.createMatch = true;
			}
			else if (arg == "--join")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.createMatch = false;
					options.joinCode = *value;
				}
			}
			else if (arg == "--match-id")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.matchId = *value;
				}
			}
			else if (arg == "--script")
			{
				if (auto value = requireValue(arg); value.has_value())
				{
					options.scriptPath = *value;
				}
			}
			else if (arg == "--interactive")
			{
				options.interactive = true;
			}
			else
			{
				addError(errors, "unknown argument: " + std::string{arg});
			}
		}
		return options;
	}

	std::vector<std::string> splitWords(const std::string& line)
	{
		std::istringstream input(line);
		std::vector<std::string> words;
		std::string word;
		while (input >> word)
		{
			words.push_back(word);
		}
		return words;
	}

	bool parseInt(std::string_view text, int& value)
	{
		if (text.empty())
		{
			return false;
		}
		std::size_t index = 0;
		int sign = 1;
		if (text[index] == '-')
		{
			sign = -1;
			++index;
		}
		if (index >= text.size())
		{
			return false;
		}
		int parsed = 0;
		for (; index < text.size(); ++index)
		{
			if (std::isdigit(static_cast<unsigned char>(text[index])) == 0)
			{
				return false;
			}
			parsed = (parsed * 10) + (text[index] - '0');
		}
		value = parsed * sign;
		return true;
	}

	std::optional<ScriptCommand> parseCommandLine(const std::string& line, std::string& error)
	{
		const auto words = splitWords(line);
		if (words.empty() || words.front().starts_with("#"))
		{
			return std::nullopt;
		}

		ScriptCommand command;
		command.kind = words[0];
		const bool needsDirection =
			command.kind == "move" || command.kind == "aim" || command.kind == "attack" || command.kind == "dash";
		if (needsDirection)
		{
			if (words.size() != 3 || !parseInt(words[1], command.dx) || !parseInt(words[2], command.dy))
			{
				error = "direction command requires: " + command.kind + " DX DY";
				return std::nullopt;
			}
			if (command.dx < -1 || command.dx > 1 || command.dy < -1 || command.dy > 1)
			{
				error = "direction values must be in -1..1";
				return std::nullopt;
			}
			command.hasDirection = true;
			return command;
		}
		if (command.kind == "interact" || command.kind == "stop")
		{
			if (words.size() != 1)
			{
				error = command.kind + " does not take arguments";
				return std::nullopt;
			}
			return command;
		}
		error = "unknown command kind: " + command.kind;
		return std::nullopt;
	}

	std::vector<ScriptCommand> defaultScenario()
	{
		return {
			{"move", -1, 0, true}, {"move", 0, -1, true}, {"move", 0, -1, true},
			{"move", 0, -1, true}, {"move", 0, -1, true}, {"move", 1, 0, true},
			{"stop", 0, 0, false}, {"interact", 0, 0, false}, {"move", -1, 0, true},
			{"move", -1, 0, true}, {"move", 0, 1, true}, {"move", 0, 1, true},
			{"move", 0, 1, true}, {"move", 0, 1, true}, {"move", 0, 1, true},
			{"move", 0, 1, true}, {"move", 1, 0, true},
		};
	}

	std::optional<std::vector<ScriptCommand>> loadScript(const std::string& path, std::vector<std::string>& errors)
	{
		if (path.empty())
		{
			return defaultScenario();
		}
		std::ifstream input(path);
		if (!input)
		{
			addError(errors, "script file not found: " + path);
			return std::nullopt;
		}
		std::vector<ScriptCommand> commands;
		std::string line;
		std::size_t lineNumber = 0;
		while (std::getline(input, line))
		{
			++lineNumber;
			std::string error;
			auto command = parseCommandLine(line, error);
			if (!error.empty())
			{
				addError(errors, path + ":" + std::to_string(lineNumber) + ": " + error);
				continue;
			}
			if (command.has_value())
			{
				commands.push_back(*command);
			}
		}
		if (commands.empty())
		{
			addError(errors, "script has no commands: " + path);
		}
		return commands;
	}

	std::string commandPayload(const CliOptions& options, const ScriptCommand& command)
	{
		std::ostringstream payload;
		payload << "{\"matchId\":" << jsonString(options.matchId) << ",\"command\":{\"kind\":" << jsonString(command.kind);
		if (command.hasDirection)
		{
			payload << ",\"direction\":{\"x\":" << command.dx << ",\"y\":" << command.dy << "}";
		}
		payload << "}}";
		return payload.str();
	}

	std::string jsonString(std::string_view value)
	{
		std::ostringstream output;
		output << '"';
		for (const char ch : value)
		{
			switch (ch)
			{
			case '\\':
				output << "\\\\";
				break;
			case '"':
				output << "\\\"";
				break;
			case '\n':
				output << "\\n";
				break;
			case '\r':
				output << "\\r";
				break;
			case '\t':
				output << "\\t";
				break;
			default:
				if (static_cast<unsigned char>(ch) < 0x20)
				{
					output << "\\u00";
					constexpr char hex[] = "0123456789abcdef";
					output << hex[(static_cast<unsigned char>(ch) >> 4u) & 0x0fu]
					       << hex[static_cast<unsigned char>(ch) & 0x0fu];
				}
				else
				{
					output << ch;
				}
				break;
			}
		}
		output << '"';
		return output.str();
	}

	std::optional<std::string> serializeValidated(Envelope envelope, ClientSessionPhase phase, std::vector<std::string>& errors)
	{
		const auto validation = validateClientEnvelope(envelope, phase);
		if (validation.code != ProtocolErrorCode::None)
		{
			addError(errors, "protocol validation failed: " + validation.message);
			return std::nullopt;
		}
		auto serialized = serializeEnvelope(envelope);
		if (!serialized.ok())
		{
			addError(errors, "protocol serialization failed: " + serialized.error.message);
			return std::nullopt;
		}
		return serialized.json;
	}

	Envelope envelope(MessageType type, std::string payload, std::uint64_t seq = 0)
	{
		Envelope result;
		result.type = type;
		result.payloadJson = std::move(payload);
		if (seq != 0)
		{
			result.sessionSeq = seq;
		}
		return result;
	}

	int failWithErrors(const std::vector<std::string>& errors)
	{
		for (const auto& error : errors)
		{
			std::cerr << "error: " << error << '\n';
		}
		return 1;
	}
}

int main(int argc, char** argv)
{
	std::vector<std::string> errors;
	const auto options = parseArgs(argc, argv, errors);
	if (!errors.empty())
	{
		return failWithErrors(errors);
	}
	if (options.help)
	{
		printHelp();
		return 0;
	}
	if (options.interactive)
	{
		addError(errors, "interactive mode is not implemented yet; use --script or default fake-connect scenario");
		return failWithErrors(errors);
	}
	if (!options.fakeConnect)
	{
		addError(errors, "real TCP mode is not implemented yet; use --fake-connect until task 0025");
		return failWithErrors(errors);
	}

	auto script = loadScript(options.scriptPath, errors);
	if (!script.has_value() || !errors.empty())
	{
		return failWithErrors(errors);
	}

	std::cout << "[fake-connect] endpoint=" << options.endpoint
	          << " transport=not-opened reason=TCP integration is task 0025\n";

	const auto auth = serializeValidated(
		envelope(MessageType::AuthRequest,
		         "{\"mode\":" + jsonString(options.authMode) + ",\"displayName\":" + jsonString(options.displayName) + "}"),
		ClientSessionPhase::Connected, errors);
	if (!auth.has_value())
	{
		return failWithErrors(errors);
	}
	std::cout << "[send] auth_request " << *auth << '\n';
	std::cout << "[event] auth_result accepted demo session (fake)\n";

	const auto matchEnvelope = options.createMatch
		? envelope(MessageType::CreateMatch, "{\"mode\":\"objective_run\",\"scenario\":\"arena_small_objective_run\"}")
		: envelope(MessageType::JoinMatch, "{\"matchCode\":" + jsonString(options.joinCode) + "}");
	const auto matchJson = serializeValidated(matchEnvelope, ClientSessionPhase::Authenticated, errors);
	if (!matchJson.has_value())
	{
		return failWithErrors(errors);
	}
	std::cout << "[send] " << (options.createMatch ? "create_match " : "join_match ") << *matchJson << '\n';
	std::cout << "[event] match_joined matchId=" << options.matchId << " code=" << options.joinCode
	          << " source=fake-connect\n";

	std::uint64_t seq = 1;
	for (const auto& command : *script)
	{
		const auto json = serializeValidated(envelope(MessageType::InputCommand, commandPayload(options, command), seq),
		                                     ClientSessionPhase::InMatch, errors);
		if (!json.has_value())
		{
			return failWithErrors(errors);
		}
		std::cout << "[send] input_command seq=" << seq << " kind=" << command.kind << ' ' << *json << '\n';
		std::cout << "[snapshot] fake pending-authoritative-update seq=" << seq
		          << " note=\"no client state mutation\"\n";
		++seq;
	}

	std::cout << "[event] script_complete commands=" << script->size()
	          << " next=run task 0025 TCP vertical slice for real backend snapshots\n";
	return 0;
}
