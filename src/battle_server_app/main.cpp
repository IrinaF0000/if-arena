#include "MatchLoop.hpp"
#include "Session.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	using if_arena::battle_backend::BackendLimits;
	using if_arena::battle_backend::MatchManager;
	using if_arena::battle_backend::SessionRegistry;

	struct TransportConfig
	{
		bool enabled{};
		std::string host;
		std::uint32_t port{};
		std::size_t maxBytes{};
		std::string path;
		bool requireTls{};
	};

	struct ServerConfig
	{
		std::string mode{"local"};
		std::uint32_t tickRate{20};
		std::uint32_t snapshotRate{10};
		std::size_t maxConnections{128};
		std::size_t maxMatches{32};
		TransportConfig tcp;
		TransportConfig websocket;
		bool demoAuthEnabled{true};
		bool telegramAuthEnabled{};
		std::string telegramBotTokenEnv;
		std::uint32_t handshakeTimeoutMs{5000};
		std::uint32_t idleTimeoutMs{30000};
		std::uint32_t maxInputCommandsPerSecond{30};
		std::size_t maxPendingWriteBytesPerSession{1024u * 1024u};
		std::size_t maxPendingOutboundMessages{64};
		std::size_t maxPendingCommandsPerSession{64};
		std::size_t maxPendingCommandsPerMatch{128};
		std::size_t maxCommandsPerTick{64};
		std::uint32_t metricsLogEverySeconds{5};
	};

	struct CliOptions
	{
		std::filesystem::path configPath{"config/examples/server.local.json"};
		bool configProvided{};
		bool localMode{};
		bool checkConfigOnly{};
		bool help{};
	};

	struct ConfigLoadResult
	{
		std::optional<ServerConfig> config;
		std::vector<std::string> errors;

		[[nodiscard]] bool ok() const
		{
			return config.has_value() && errors.empty();
		}
	};

	bool isSpace(char value)
	{
		return std::isspace(static_cast<unsigned char>(value)) != 0;
	}

	std::size_t skipWhitespace(std::string_view text, std::size_t offset)
	{
		while (offset < text.size() && isSpace(text[offset]))
		{
			++offset;
		}
		return offset;
	}

	std::string trimCopy(std::string_view text)
	{
		std::size_t begin = 0;
		while (begin < text.size() && isSpace(text[begin]))
		{
			++begin;
		}
		std::size_t end = text.size();
		while (end > begin && isSpace(text[end - 1]))
		{
			--end;
		}
		return std::string{text.substr(begin, end - begin)};
	}

	std::optional<std::size_t> findStringEnd(std::string_view text, std::size_t quote)
	{
		bool escaped = false;
		for (std::size_t index = quote + 1; index < text.size(); ++index)
		{
			if (escaped)
			{
				escaped = false;
				continue;
			}
			if (text[index] == '\\')
			{
				escaped = true;
				continue;
			}
			if (text[index] == '"')
			{
				return index;
			}
		}
		return std::nullopt;
	}

	std::optional<std::size_t> findObjectEnd(std::string_view text, std::size_t openBrace)
	{
		int depth = 0;
		for (std::size_t index = openBrace; index < text.size(); ++index)
		{
			if (text[index] == '"')
			{
				const auto stringEnd = findStringEnd(text, index);
				if (!stringEnd.has_value())
				{
					return std::nullopt;
				}
				index = *stringEnd;
				continue;
			}
			if (text[index] == '{')
			{
				++depth;
			}
			if (text[index] == '}')
			{
				--depth;
				if (depth == 0)
				{
					return index;
				}
			}
		}
		return std::nullopt;
	}

	std::optional<std::size_t> findKey(std::string_view object, std::string_view key)
	{
		const std::string needle = "\"" + std::string{key} + "\"";
		std::size_t searchFrom = 0;
		while (searchFrom < object.size())
		{
			const auto found = object.find(needle, searchFrom);
			if (found == std::string_view::npos)
			{
				return std::nullopt;
			}
			const auto afterKey = skipWhitespace(object, found + needle.size());
			if (afterKey < object.size() && object[afterKey] == ':')
			{
				return skipWhitespace(object, afterKey + 1);
			}
			searchFrom = found + needle.size();
		}
		return std::nullopt;
	}

	std::optional<std::string_view> objectField(std::string_view object, std::string_view key)
	{
		const auto valueStart = findKey(object, key);
		if (!valueStart.has_value() || *valueStart >= object.size() || object[*valueStart] != '{')
		{
			return std::nullopt;
		}
		const auto valueEnd = findObjectEnd(object, *valueStart);
		if (!valueEnd.has_value())
		{
			return std::nullopt;
		}
		return object.substr(*valueStart, *valueEnd - *valueStart + 1);
	}

	std::optional<std::string_view> rawField(std::string_view object, std::string_view key)
	{
		const auto valueStart = findKey(object, key);
		if (!valueStart.has_value())
		{
			return std::nullopt;
		}

		if (*valueStart < object.size() && object[*valueStart] == '"')
		{
			const auto valueEnd = findStringEnd(object, *valueStart);
			if (!valueEnd.has_value())
			{
				return std::nullopt;
			}
			return object.substr(*valueStart, *valueEnd - *valueStart + 1);
		}

		std::size_t valueEnd = *valueStart;
		while (valueEnd < object.size() && object[valueEnd] != ',' && object[valueEnd] != '}')
		{
			++valueEnd;
		}
		return object.substr(*valueStart, valueEnd - *valueStart);
	}

	std::optional<std::string> stringField(std::string_view object, std::string_view key)
	{
		const auto raw = rawField(object, key);
		if (!raw.has_value() || raw->size() < 2 || raw->front() != '"' || raw->back() != '"')
		{
			return std::nullopt;
		}
		std::string output;
		bool escaped = false;
		for (std::size_t index = 1; index + 1 < raw->size(); ++index)
		{
			const char ch = (*raw)[index];
			if (escaped)
			{
				output.push_back(ch);
				escaped = false;
				continue;
			}
			if (ch == '\\')
			{
				escaped = true;
				continue;
			}
			output.push_back(ch);
		}
		return output;
	}

	std::optional<bool> boolField(std::string_view object, std::string_view key)
	{
		const auto raw = rawField(object, key);
		if (!raw.has_value())
		{
			return std::nullopt;
		}
		const auto value = trimCopy(*raw);
		if (value == "true")
		{
			return true;
		}
		if (value == "false")
		{
			return false;
		}
		return std::nullopt;
	}

	std::optional<std::uint64_t> uintField(std::string_view object, std::string_view key)
	{
		const auto raw = rawField(object, key);
		if (!raw.has_value())
		{
			return std::nullopt;
		}
		std::uint64_t value = 0;
		const auto trimmed = trimCopy(*raw);
		if (trimmed.empty() || !std::all_of(trimmed.begin(), trimmed.end(), [](char ch) {
			    return std::isdigit(static_cast<unsigned char>(ch)) != 0;
		    }))
		{
			return std::nullopt;
		}
		for (const char ch : trimmed)
		{
			value = (value * 10u) + static_cast<std::uint64_t>(ch - '0');
		}
		return value;
	}

	void addError(std::vector<std::string>& errors, std::string message)
	{
		errors.push_back(std::move(message));
	}

	std::string readFile(const std::filesystem::path& path)
	{
		std::ifstream input(path);
		std::ostringstream buffer;
		buffer << input.rdbuf();
		return buffer.str();
	}

	std::uint32_t readUint32(std::string_view object, std::string_view key, std::uint32_t fallback,
	                         std::vector<std::string>& errors)
	{
		const auto value = uintField(object, key);
		if (!value.has_value())
		{
			addError(errors, "missing or invalid numeric config field: " + std::string{key});
			return fallback;
		}
		if (*value > UINT32_MAX)
		{
			addError(errors, "numeric config field exceeds uint32: " + std::string{key});
			return fallback;
		}
		return static_cast<std::uint32_t>(*value);
	}

	std::size_t readSize(std::string_view object, std::string_view key, std::size_t fallback,
	                     std::vector<std::string>& errors)
	{
		const auto value = uintField(object, key);
		if (!value.has_value())
		{
			addError(errors, "missing or invalid numeric config field: " + std::string{key});
			return fallback;
		}
		return static_cast<std::size_t>(*value);
	}

	bool readBool(std::string_view object, std::string_view key, bool fallback, std::vector<std::string>& errors)
	{
		const auto value = boolField(object, key);
		if (!value.has_value())
		{
			addError(errors, "missing or invalid boolean config field: " + std::string{key});
			return fallback;
		}
		return *value;
	}

	std::string readString(std::string_view object, std::string_view key, std::string fallback,
	                       std::vector<std::string>& errors)
	{
		const auto value = stringField(object, key);
		if (!value.has_value())
		{
			addError(errors, "missing or invalid string config field: " + std::string{key});
			return fallback;
		}
		return *value;
	}

	TransportConfig readTcpConfig(std::string_view transports, std::vector<std::string>& errors)
	{
		TransportConfig config;
		const auto tcp = objectField(transports, "tcp");
		if (!tcp.has_value())
		{
			addError(errors, "missing transports.tcp object");
			return config;
		}
		config.enabled = readBool(*tcp, "enabled", false, errors);
		config.host = readString(*tcp, "host", "127.0.0.1", errors);
		config.port = readUint32(*tcp, "port", 5555, errors);
		config.maxBytes = readSize(*tcp, "maxFrameBytes", 64u * 1024u, errors);
		return config;
	}

	TransportConfig readWebSocketConfig(std::string_view transports, std::vector<std::string>& errors)
	{
		TransportConfig config;
		const auto websocket = objectField(transports, "websocket");
		if (!websocket.has_value())
		{
			addError(errors, "missing transports.websocket object");
			return config;
		}
		config.enabled = readBool(*websocket, "enabled", false, errors);
		config.host = readString(*websocket, "host", "127.0.0.1", errors);
		config.port = readUint32(*websocket, "port", 8081, errors);
		config.path = readString(*websocket, "path", "/ws", errors);
		config.maxBytes = readSize(*websocket, "maxMessageBytes", 64u * 1024u, errors);
		config.requireTls = readBool(*websocket, "requireTls", false, errors);
		return config;
	}

	void validateConfig(const ServerConfig& config, std::vector<std::string>& errors)
	{
		if (config.mode != "local" && config.mode != "public")
		{
			addError(errors, "mode must be either local or public");
		}
		if (config.tickRate == 0 || config.tickRate > 240)
		{
			addError(errors, "server.tickRate must be in range 1..240");
		}
		if (config.snapshotRate == 0 || config.snapshotRate > config.tickRate)
		{
			addError(errors, "server.snapshotRate must be in range 1..tickRate");
		}
		if (config.maxConnections == 0 || config.maxConnections > 100000)
		{
			addError(errors, "server.maxConnections must be in range 1..100000");
		}
		if (config.maxMatches == 0 || config.maxMatches > 10000)
		{
			addError(errors, "server.maxMatches must be in range 1..10000");
		}
		const auto validatePort = [&errors](const char* name, const TransportConfig& transport) {
			if (transport.enabled && (transport.port == 0 || transport.port > 65535))
			{
				addError(errors, std::string{name} + ".port must be in range 1..65535");
			}
			if (transport.enabled && transport.host.empty())
			{
				addError(errors, std::string{name} + ".host must not be empty");
			}
			if (transport.enabled && (transport.maxBytes == 0 || transport.maxBytes > 1024u * 1024u))
			{
				addError(errors, std::string{name} + " max message/frame bytes must be in range 1..1048576");
			}
		};
		validatePort("transports.tcp", config.tcp);
		validatePort("transports.websocket", config.websocket);
		if (config.websocket.enabled && config.websocket.path.empty())
		{
			addError(errors, "transports.websocket.path must not be empty");
		}
		if (config.handshakeTimeoutMs == 0 || config.idleTimeoutMs == 0)
		{
			addError(errors, "security timeouts must be positive");
		}
		if (config.maxInputCommandsPerSecond == 0 || config.maxInputCommandsPerSecond > 240)
		{
			addError(errors, "security.maxInputCommandsPerSecond must be in range 1..240");
		}
		if (config.maxPendingWriteBytesPerSession == 0 || config.maxPendingWriteBytesPerSession > 64u * 1024u * 1024u)
		{
			addError(errors, "security.maxPendingWriteBytesPerSession must be in range 1..67108864");
		}
		if (config.maxPendingOutboundMessages == 0 || config.maxPendingOutboundMessages > 4096)
		{
			addError(errors, "security.maxPendingOutboundMessages must be in range 1..4096");
		}
		if (config.maxPendingCommandsPerSession == 0 || config.maxPendingCommandsPerSession > 4096)
		{
			addError(errors, "security.maxPendingCommandsPerSession must be in range 1..4096");
		}
		if (config.maxPendingCommandsPerMatch == 0 || config.maxPendingCommandsPerMatch > 65536)
		{
			addError(errors, "security.maxPendingCommandsPerMatch must be in range 1..65536");
		}
		if (config.maxCommandsPerTick == 0 || config.maxCommandsPerTick > 4096)
		{
			addError(errors, "security.maxCommandsPerTick must be in range 1..4096");
		}
		if (config.metricsLogEverySeconds == 0 || config.metricsLogEverySeconds > 3600)
		{
			addError(errors, "metrics.logEverySeconds must be in range 1..3600");
		}
		if (config.mode == "public")
		{
			if (config.demoAuthEnabled)
			{
				addError(errors, "public config must disable security.demoAuthEnabled");
			}
			if (config.websocket.enabled && !config.websocket.requireTls)
			{
				addError(errors, "public websocket config must require TLS/WSS");
			}
			if (config.telegramAuthEnabled && config.telegramBotTokenEnv.empty())
			{
				addError(errors, "public Telegram auth must reference a token environment variable name");
			}
		}
	}

	ConfigLoadResult loadConfig(const std::filesystem::path& path)
	{
		ConfigLoadResult result;
		if (!std::filesystem::exists(path))
		{
			addError(result.errors, "config file not found: " + path.string());
			return result;
		}

		const std::string json = readFile(path);
		if (json.empty())
		{
			addError(result.errors, "config file is empty: " + path.string());
			return result;
		}

		ServerConfig config;
		config.mode = stringField(json, "mode").value_or("local");

		const auto server = objectField(json, "server");
		const auto transports = objectField(json, "transports");
		const auto security = objectField(json, "security");
		const auto metrics = objectField(json, "metrics");
		if (!server.has_value())
		{
			addError(result.errors, "missing server object");
		}
		if (!transports.has_value())
		{
			addError(result.errors, "missing transports object");
		}
		if (!security.has_value())
		{
			addError(result.errors, "missing security object");
		}
		if (!metrics.has_value())
		{
			addError(result.errors, "missing metrics object");
		}
		if (!result.errors.empty())
		{
			return result;
		}

		config.tickRate = readUint32(*server, "tickRate", config.tickRate, result.errors);
		config.snapshotRate = readUint32(*server, "snapshotRate", config.snapshotRate, result.errors);
		config.maxConnections = readSize(*server, "maxConnections", config.maxConnections, result.errors);
		config.maxMatches = readSize(*server, "maxMatches", config.maxMatches, result.errors);
		config.tcp = readTcpConfig(*transports, result.errors);
		config.websocket = readWebSocketConfig(*transports, result.errors);
		config.demoAuthEnabled = readBool(*security, "demoAuthEnabled", config.demoAuthEnabled, result.errors);
		config.telegramAuthEnabled =
			readBool(*security, "telegramAuthEnabled", config.telegramAuthEnabled, result.errors);
		config.telegramBotTokenEnv = stringField(*security, "telegramBotTokenEnv").value_or("");
		config.handshakeTimeoutMs =
			readUint32(*security, "handshakeTimeoutMs", config.handshakeTimeoutMs, result.errors);
		config.idleTimeoutMs = readUint32(*security, "idleTimeoutMs", config.idleTimeoutMs, result.errors);
		config.maxInputCommandsPerSecond =
			readUint32(*security, "maxInputCommandsPerSecond", config.maxInputCommandsPerSecond, result.errors);
		config.maxPendingWriteBytesPerSession =
			readSize(*security, "maxPendingWriteBytesPerSession", config.maxPendingWriteBytesPerSession, result.errors);
		config.maxPendingOutboundMessages =
			readSize(*security, "maxPendingOutboundMessages", config.maxPendingOutboundMessages, result.errors);
		config.maxPendingCommandsPerSession =
			readSize(*security, "maxPendingCommandsPerSession", config.maxPendingCommandsPerSession, result.errors);
		config.maxPendingCommandsPerMatch =
			readSize(*security, "maxPendingCommandsPerMatch", config.maxPendingCommandsPerMatch, result.errors);
		config.maxCommandsPerTick = readSize(*security, "maxCommandsPerTick", config.maxCommandsPerTick, result.errors);
		config.metricsLogEverySeconds =
			readUint32(*metrics, "logEverySeconds", config.metricsLogEverySeconds, result.errors);

		validateConfig(config, result.errors);
		if (result.errors.empty())
		{
			result.config = config;
		}
		return result;
	}

	BackendLimits backendLimitsFrom(const ServerConfig& config)
	{
		BackendLimits limits;
		limits.maxSessions = config.maxConnections;
		limits.maxMatches = config.maxMatches;
		limits.maxPendingOutboundMessages = config.maxPendingOutboundMessages;
		limits.maxPendingOutboundBytes = config.maxPendingWriteBytesPerSession;
		limits.maxPendingCommandsPerSession = config.maxPendingCommandsPerSession;
		limits.maxPendingCommandsPerMatch = config.maxPendingCommandsPerMatch;
		limits.maxCommandsPerTick = config.maxCommandsPerTick;
		limits.maxCommandsPerSessionPerTick =
			std::max<std::size_t>(1, config.maxInputCommandsPerSecond / std::max<std::uint32_t>(1, config.tickRate));
		return limits;
	}

	CliOptions parseArgs(int argc, char** argv, std::vector<std::string>& errors)
	{
		CliOptions options;
		for (int index = 1; index < argc; ++index)
		{
			const std::string_view arg{argv[index]};
			if (arg == "--help" || arg == "-h")
			{
				options.help = true;
			}
			else if (arg == "--local")
			{
				options.localMode = true;
			}
			else if (arg == "--check-config")
			{
				options.checkConfigOnly = true;
			}
			else if (arg == "--config")
			{
				if (index + 1 >= argc)
				{
					addError(errors, "--config requires a path");
					break;
				}
				options.configPath = argv[++index];
				options.configProvided = true;
			}
			else
			{
				addError(errors, "unknown argument: " + std::string{arg});
			}
		}
		return options;
	}

	void printHelp()
	{
		std::cout << "Usage: battle_server_app [--config PATH] [--local] [--check-config]\n"
		          << "\n"
		          << "Loads IF Arena server config, initializes backend limits, and starts enabled transports when "
		             "listener implementations are available.\n";
	}

	void logStartup(const ServerConfig& config, const BackendLimits& limits)
	{
		std::cout << "IF Arena server config loaded"
		          << " mode=" << config.mode << " tickRate=" << config.tickRate
		          << " snapshotRate=" << config.snapshotRate << " maxConnections=" << limits.maxSessions
		          << " maxMatches=" << limits.maxMatches << " metricsEverySeconds=" << config.metricsLogEverySeconds
		          << '\n';
		std::cout << "Configured transports:"
		          << " tcp=" << (config.tcp.enabled ? "enabled" : "disabled")
		          << " ws=" << (config.websocket.enabled ? "enabled" : "disabled") << '\n';
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
	std::vector<std::string> argErrors;
	auto options = parseArgs(argc, argv, argErrors);
	if (!argErrors.empty())
	{
		return failWithErrors(argErrors);
	}
	if (options.help)
	{
		printHelp();
		return 0;
	}
	if (options.localMode && !options.configProvided)
	{
		options.configPath = "config/examples/server.local.json";
	}

	const auto loaded = loadConfig(options.configPath);
	if (!loaded.ok())
	{
		return failWithErrors(loaded.errors);
	}

	const auto& config = *loaded.config;
	const auto limits = backendLimitsFrom(config);
	SessionRegistry sessions{limits};
	MatchManager matches{sessions, limits};
	logStartup(config, limits);
	std::cout << "Backend initialized activeSessions=" << sessions.metrics().activeSessions
	          << " activeMatches=" << matches.metrics().activeMatches << '\n';

	if (options.checkConfigOnly)
	{
		std::cout << "Config validation succeeded. Transport startup skipped by --check-config.\n";
		return 0;
	}

	std::vector<std::string> startupErrors;
	if (config.tcp.enabled)
	{
		addError(startupErrors, "TCP listener is not implemented in this task yet; disable transports.tcp.enabled or run task 0025+.");
	}
	if (config.websocket.enabled)
	{
		addError(startupErrors,
		         "WebSocket listener is not implemented in this task yet; disable transports.websocket.enabled or run task 0026+.");
	}
	if (!config.tcp.enabled && !config.websocket.enabled)
	{
		addError(startupErrors, "no transports are enabled; server has no listener to run");
	}
	if (!startupErrors.empty())
	{
		std::cout << "Server startup stopped before accepting clients. Shutdown complete.\n";
		return failWithErrors(startupErrors);
	}

	std::cout << "Server started. Press Ctrl+C to stop.\n";
	std::cout << "Shutdown complete.\n";
	return 0;
}
