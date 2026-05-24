#include "MatchLoop.hpp"
#include "Protocol.hpp"
#include "Session.hpp"
#include "TcpFrameCodec.hpp"
#include "WebSocketSession.hpp"
#include "security/TelegramAuth.hpp"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdlib>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace
{
	using if_arena::battle_backend::BackendLimits;
	using if_arena::battle_backend::BackendCommand;
	using if_arena::battle_backend::BackendCommandKind;
	using if_arena::battle_backend::ConnectionId;
	using if_arena::battle_backend::DisconnectReason;
	using if_arena::battle_backend::IOutboundSession;
	using if_arena::battle_backend::MatchManager;
	using if_arena::battle_backend::MatchId;
	using if_arena::battle_backend::PlayerId;
	using if_arena::battle_backend::SessionId;
	using if_arena::battle_backend::SessionRegistry;
	using if_arena::battle_protocol::ClientSessionPhase;
	using if_arena::battle_protocol::Envelope;
	using if_arena::battle_protocol::MessageType;
	using if_arena::battle_protocol::ProtocolErrorCode;
	using if_arena::battle_protocol::parseEnvelope;
	using if_arena::battle_protocol::serializeEnvelope;
	using if_arena::battle_protocol::validateClientEnvelope;
	using if_arena::battle_transport_tcp::TcpConnection;
	using if_arena::battle_transport_tcp::TcpEndpoint;
	using if_arena::battle_transport_tcp::TcpListener;
	using if_arena::battle_transport_tcp::TcpReadStatus;
	using if_arena::battle_transport_ws::WebSocketConnection;
	using if_arena::battle_transport_ws::WebSocketEndpoint;
	using if_arena::battle_transport_ws::WebSocketListener;
	using if_arena::battle_transport_ws::WebSocketReadStatus;
	using if_arena::battle_transport_ws::WebSocketSessionAdapter;

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
		std::size_t maxClients{};
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

	std::optional<int> intField(std::string_view object, std::string_view key)
	{
		const auto raw = rawField(object, key);
		if (!raw.has_value())
		{
			return std::nullopt;
		}
		const auto trimmed = trimCopy(*raw);
		if (trimmed.empty())
		{
			return std::nullopt;
		}
		std::size_t index = 0;
		int sign = 1;
		if (trimmed[index] == '-')
		{
			sign = -1;
			++index;
		}
		if (index >= trimmed.size())
		{
			return std::nullopt;
		}
		int value = 0;
		for (; index < trimmed.size(); ++index)
		{
			if (std::isdigit(static_cast<unsigned char>(trimmed[index])) == 0)
			{
				return std::nullopt;
			}
			value = (value * 10) + (trimmed[index] - '0');
		}
		return value * sign;
	}

	std::optional<std::uint64_t> parseUint64Text(std::string_view text)
	{
		if (text.empty())
		{
			return std::nullopt;
		}
		std::uint64_t value = 0;
		for (const char ch : text)
		{
			if (std::isdigit(static_cast<unsigned char>(ch)) == 0)
			{
				return std::nullopt;
			}
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
			else if (arg == "--max-clients")
			{
				if (index + 1 >= argc)
				{
					addError(errors, "--max-clients requires a value");
					break;
				}
				const auto parsed = parseUint64Text(argv[++index]);
				if (!parsed.has_value())
				{
					addError(errors, "--max-clients must be a non-negative integer");
				}
				else
				{
					options.maxClients = static_cast<std::size_t>(*parsed);
				}
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
		std::cout << "Usage: battle_server_app [--config PATH] [--local] [--check-config] [--max-clients N]\n"
		          << "\n"
		          << "Loads IF Arena server config, initializes backend limits, and starts the local TCP listener.\n";
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

	struct TcpRuntimeState
	{
		SessionRegistry& sessions;
		MatchManager& matches;
		BackendLimits limits;
		std::mutex mutex;
		std::uint64_t nextConnectionId{1};
		std::vector<SessionId> liveSessions;
		struct ActiveMatchTick
		{
			MatchId match;
			std::chrono::steady_clock::time_point nextTickAt;
			std::chrono::steady_clock::time_point nextSnapshotAt;
		};
		std::vector<ActiveMatchTick> activeMatchTicks;
		std::chrono::milliseconds tickInterval{50};
		std::chrono::milliseconds snapshotInterval{100};
		std::chrono::milliseconds handshakeTimeout{5000};
		std::chrono::milliseconds idleTimeout{30000};
	};

	class TcpBackendOutbound final : public IOutboundSession
	{
	public:
		explicit TcpBackendOutbound(TcpConnection& connection)
			: _connection(connection)
		{
		}

		bool send(std::string_view payload) override
		{
			std::lock_guard lock(_mutex);
			return _connection.sendFrame(payload).ok;
		}

		void close(DisconnectReason) override
		{
			std::lock_guard lock(_mutex);
			_connection.close();
		}

	private:
		TcpConnection& _connection;
		std::mutex _mutex;
	};

	class WebSocketBackendOutbound final : public IOutboundSession
	{
	public:
		explicit WebSocketBackendOutbound(WebSocketConnection& connection)
			: _connection(connection)
		{
		}

		bool send(std::string_view payload) override
		{
			std::lock_guard lock(_mutex);
			return _connection.sendText(payload).ok;
		}

		void close(DisconnectReason) override
		{
			std::lock_guard lock(_mutex);
			_connection.close();
		}

	private:
		WebSocketConnection& _connection;
		std::mutex _mutex;
	};

	std::string backendReasonName(if_arena::battle_backend::BackendRejectReason reason)
	{
		using if_arena::battle_backend::BackendRejectReason;
		switch (reason)
		{
		case BackendRejectReason::None:
			return "none";
		case BackendRejectReason::AuthRequired:
			return "auth_required";
		case BackendRejectReason::Closed:
			return "closed";
		case BackendRejectReason::QueueFull:
			return "queue_full";
		case BackendRejectReason::CapacityReached:
			return "capacity_reached";
		case BackendRejectReason::NotFound:
			return "not_found";
		case BackendRejectReason::InvalidMatch:
			return "invalid_match";
		case BackendRejectReason::MatchNotStarted:
			return "match_not_started";
		case BackendRejectReason::MatchFull:
			return "match_full";
		case BackendRejectReason::InvalidOwnership:
			return "invalid_ownership";
		case BackendRejectReason::InvalidSequence:
			return "invalid_sequence";
		case BackendRejectReason::RateLimited:
			return "rate_limited";
		}
		return "unknown";
	}

	Envelope serverEnvelope(MessageType type, std::string payload)
	{
		Envelope envelope;
		envelope.type = type;
		envelope.payloadJson = std::move(payload);
		return envelope;
	}

	bool sendServerEnvelope(TcpConnection& connection, MessageType type, std::string payload)
	{
		const auto serialized = serializeEnvelope(serverEnvelope(type, std::move(payload)));
		if (!serialized.ok())
		{
			return false;
		}
		return connection.sendFrame(*serialized.json).ok;
	}

	bool sendError(TcpConnection& connection, std::string code, std::string message)
	{
		return sendServerEnvelope(connection, MessageType::Error,
		                          "{\"code\":" + jsonString(code) + ",\"message\":" + jsonString(message) + "}");
	}

	bool sendServerEnvelope(WebSocketConnection& connection, MessageType type, std::string payload)
	{
		const auto serialized = serializeEnvelope(serverEnvelope(type, std::move(payload)));
		if (!serialized.ok())
		{
			return false;
		}
		return connection.sendText(*serialized.json).ok;
	}

	bool sendError(WebSocketConnection& connection, std::string code, std::string message)
	{
		return sendServerEnvelope(connection, MessageType::Error,
		                          "{\"code\":" + jsonString(code) + ",\"message\":" + jsonString(message) + "}");
	}

	template <typename RuntimeState>
	void flushAllSessions(RuntimeState& state)
	{
		for (const auto sessionId : state.liveSessions)
		{
			if (auto* session = state.sessions.find(sessionId); session != nullptr)
			{
				session->flushOutbound();
			}
		}
	}

	void activateTcpMatchTicker(TcpRuntimeState& state, MatchId match, std::chrono::steady_clock::time_point now)
	{
		const auto found = std::find_if(state.activeMatchTicks.begin(), state.activeMatchTicks.end(),
		                                [match](const auto& active) { return active.match.value == match.value; });
		if (found != state.activeMatchTicks.end())
		{
			return;
		}
		state.activeMatchTicks.push_back(
		    TcpRuntimeState::ActiveMatchTick{match, now + state.tickInterval, now + state.snapshotInterval});
	}

	void advanceDueTcpMatches(TcpRuntimeState& state, std::chrono::steady_clock::time_point now)
	{
		bool ticked = false;
		for (auto& active : state.activeMatchTicks)
		{
			std::size_t catchUpTicks = 0;
			while (now >= active.nextTickAt && catchUpTicks < 4)
			{
				const bool broadcastSnapshot = active.nextTickAt >= active.nextSnapshotAt;
				const auto result = state.matches.tick(active.match, broadcastSnapshot);
				ticked = ticked || result.accepted;
				if (broadcastSnapshot)
				{
					do
					{
						active.nextSnapshotAt += state.snapshotInterval;
					} while (active.nextSnapshotAt <= active.nextTickAt);
				}
				active.nextTickAt += state.tickInterval;
				++catchUpTicks;
			}
			if (now >= active.nextTickAt)
			{
				active.nextTickAt = now + state.tickInterval;
				if (now >= active.nextSnapshotAt)
				{
					active.nextSnapshotAt = now + state.snapshotInterval;
				}
			}
		}
		if (ticked)
		{
			flushAllSessions(state);
		}
	}

	std::optional<BackendCommandKind> commandKindFrom(std::string_view kind)
	{
		if (kind == "move" || kind == "aim")
		{
			return BackendCommandKind::Move;
		}
		if (kind == "stop")
		{
			return BackendCommandKind::Stop;
		}
		if (kind == "attack")
		{
			return BackendCommandKind::Attack;
		}
		if (kind == "interact")
		{
			return BackendCommandKind::Interact;
		}
		if (kind == "dash")
		{
			return BackendCommandKind::Dash;
		}
		return std::nullopt;
	}

	std::optional<BackendCommand> parseBackendCommand(const Envelope& envelope, MatchId& match)
	{
		const auto matchId = stringField(envelope.payloadJson, "matchId");
		if (!matchId.has_value())
		{
			return std::nullopt;
		}
		const auto parsedMatch = parseUint64Text(*matchId);
		if (!parsedMatch.has_value())
		{
			return std::nullopt;
		}
		match = MatchId{*parsedMatch};

		const auto command = objectField(envelope.payloadJson, "command");
		if (!command.has_value())
		{
			return std::nullopt;
		}
		const auto kind = stringField(*command, "kind");
		if (!kind.has_value())
		{
			return std::nullopt;
		}
		const auto backendKind = commandKindFrom(*kind);
		if (!backendKind.has_value())
		{
			return std::nullopt;
		}

		BackendCommand result;
		result.kind = *backendKind;
		const auto direction = objectField(*command, "direction");
		if (direction.has_value())
		{
			const auto x = intField(*direction, "x");
			const auto y = intField(*direction, "y");
			if (!x.has_value() || !y.has_value())
			{
				return std::nullopt;
			}
			result.direction = if_arena::battle_core::Direction{*x, *y};
		}
		return result;
	}

	void handleTcpClient(TcpConnection connection, TcpRuntimeState& state)
	{
		TcpBackendOutbound outbound{connection};
		SessionId sessionId{};
		{
			std::lock_guard lock(state.mutex);
			const auto created = state.sessions.createSession(ConnectionId{state.nextConnectionId++}, outbound);
			if (!created.accepted() || !created.session.has_value())
			{
				sendError(connection, "capacity_reached", "server session capacity reached");
				return;
			}
			sessionId = *created.session;
			state.liveSessions.push_back(sessionId);
		}

		ClientSessionPhase phase = ClientSessionPhase::Connected;
		std::optional<MatchId> currentMatch;
		const auto connectedAt = std::chrono::steady_clock::now();
		auto lastActivityAt = connectedAt;
		bool idlePingOutstanding = false;
		while (connection.valid())
		{
			auto read = connection.readFrame();
			if (read.status == TcpReadStatus::TimedOut)
			{
				const auto now = std::chrono::steady_clock::now();
				{
					std::lock_guard lock(state.mutex);
					advanceDueTcpMatches(state, now);
				}
				if (phase == ClientSessionPhase::Connected && now - connectedAt >= state.handshakeTimeout)
				{
					sendError(connection, "handshake_timeout", "authentication handshake timed out");
					break;
				}
				if (now - lastActivityAt >= state.idleTimeout)
				{
					if (idlePingOutstanding)
					{
						sendError(connection, "idle_timeout", "connection idle timeout");
						break;
					}
					sendServerEnvelope(connection, MessageType::Ping, "{}");
					idlePingOutstanding = true;
					lastActivityAt = now;
				}
				continue;
			}
			if (read.status == TcpReadStatus::Closed)
			{
				break;
			}
			if (read.status == TcpReadStatus::Error)
			{
				break;
			}

			lastActivityAt = std::chrono::steady_clock::now();
			idlePingOutstanding = false;
			auto parsed = parseEnvelope(read.frame);
			if (!parsed.ok())
			{
				sendError(connection, "protocol_error", "invalid protocol envelope");
				break;
			}
			const auto validation = validateClientEnvelope(*parsed.envelope, phase);
			if (validation.code != ProtocolErrorCode::None)
			{
				sendError(connection, "protocol_error", validation.message);
				break;
			}

			if (parsed.envelope->type == MessageType::AuthRequest)
			{
				std::lock_guard lock(state.mutex);
				auto* session = state.sessions.find(sessionId);
				if (session == nullptr || !session->authenticate(PlayerId{sessionId.value}).accepted)
				{
					sendError(connection, "auth_failed", "backend authentication failed");
					break;
				}
				phase = ClientSessionPhase::Authenticated;
				sendServerEnvelope(connection, MessageType::AuthResult,
				                   "{\"accepted\":true,\"sessionId\":\"" + std::to_string(sessionId.value) + "\"}");
				continue;
			}

			if (parsed.envelope->type == MessageType::CreateMatch)
			{
				std::lock_guard lock(state.mutex);
				const auto created = state.matches.createMatch(sessionId);
				if (!created.accepted() || !created.match.has_value())
				{
					sendError(connection, "create_match_rejected", backendReasonName(created.result.reason));
					continue;
				}
				currentMatch = *created.match;
				phase = ClientSessionPhase::InMatch;
				sendServerEnvelope(connection, MessageType::MatchJoined,
				                   "{\"matchId\":\"" + std::to_string(created.match->value) + "\",\"matchCode\":\"" +
				                    created.joinCode + "\"}");
				continue;
			}

			if (parsed.envelope->type == MessageType::JoinMatch)
			{
				const auto joinCode = stringField(parsed.envelope->payloadJson, "matchCode");
				if (!joinCode.has_value())
				{
					sendError(connection, "join_match_rejected", "missing match code");
					continue;
				}
				std::lock_guard lock(state.mutex);
				const auto joined = state.matches.joinMatch(sessionId, *joinCode);
				if (!joined.accepted() || !joined.match.has_value())
				{
					sendError(connection, "join_match_rejected", backendReasonName(joined.result.reason));
					continue;
				}
				currentMatch = *joined.match;
				phase = ClientSessionPhase::InMatch;
				sendServerEnvelope(connection, MessageType::MatchJoined,
				                   "{\"matchId\":\"" + std::to_string(joined.match->value) + "\",\"matchCode\":\"" +
				                    *joinCode + "\"}");
				activateTcpMatchTicker(state, *currentMatch, std::chrono::steady_clock::now());
				state.matches.tick(*currentMatch);
				flushAllSessions(state);
				continue;
			}

			if (parsed.envelope->type == MessageType::InputCommand)
			{
				MatchId match{};
				const auto command = parseBackendCommand(*parsed.envelope, match);
				if (!command.has_value() || !parsed.envelope->sessionSeq.has_value())
				{
					sendError(connection, "input_rejected", "invalid input command payload");
					continue;
				}
				std::lock_guard lock(state.mutex);
				const auto submitted = state.matches.submitCommand(sessionId, match, *parsed.envelope->sessionSeq, *command);
				sendServerEnvelope(connection, MessageType::InputAck,
				                   "{\"accepted\":" + std::string{submitted.accepted ? "true" : "false"} +
				                    ",\"reason\":\"" + backendReasonName(submitted.reason) + "\"}");
				if (submitted.accepted)
				{
					advanceDueTcpMatches(state, std::chrono::steady_clock::now());
				}
				continue;
			}

			if (parsed.envelope->type == MessageType::Ping)
			{
				sendServerEnvelope(connection, MessageType::Pong, "{}");
				continue;
			}
			if (parsed.envelope->type == MessageType::Pong)
			{
				continue;
			}
		}

		{
			std::lock_guard lock(state.mutex);
			state.matches.disconnect(sessionId, DisconnectReason::ClientClosed);
		}
	}

	bool validateWebSocketAuth(const ServerConfig& config, const Envelope& envelope)
	{
		const auto mode = stringField(envelope.payloadJson, "mode");
		if (!mode.has_value())
		{
			return false;
		}
		if (*mode == "demo")
		{
			return config.demoAuthEnabled;
		}
		if (*mode != "telegram" || !config.telegramAuthEnabled || config.telegramBotTokenEnv.empty())
		{
			return false;
		}
		const auto initData = stringField(envelope.payloadJson, "initData");
		if (!initData.has_value())
		{
			return false;
		}
		const char* token = std::getenv(config.telegramBotTokenEnv.c_str());
		if (token == nullptr || std::string_view{token}.empty())
		{
			return false;
		}
		const auto now = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::seconds>(
		                                                std::chrono::system_clock::now().time_since_epoch())
		                                                .count());
		const if_arena::battle_backend::security::TelegramAuthValidator validator{
			if_arena::battle_backend::security::TelegramAuthConfig{
				token,
				now,
				300,
				4096,
			},
		};
		return validator.validate(*initData).ok();
	}

	struct WebSocketRuntimeState
	{
		SessionRegistry& sessions;
		MatchManager& matches;
		BackendLimits limits;
		const ServerConfig& config;
		std::mutex mutex;
		std::uint64_t nextConnectionId{1};
		std::vector<SessionId> liveSessions;
		struct ActiveMatchTick
		{
			MatchId match;
			std::chrono::steady_clock::time_point nextTickAt;
			std::chrono::steady_clock::time_point nextSnapshotAt;
		};
		std::vector<ActiveMatchTick> activeMatchTicks;
		std::chrono::milliseconds tickInterval{50};
		std::chrono::milliseconds snapshotInterval{100};
	};

	void activateWebSocketMatchTicker(WebSocketRuntimeState& state, MatchId match, std::chrono::steady_clock::time_point now)
	{
		const auto found = std::find_if(state.activeMatchTicks.begin(), state.activeMatchTicks.end(),
		                                [match](const auto& active) { return active.match.value == match.value; });
		if (found != state.activeMatchTicks.end())
		{
			return;
		}
		state.activeMatchTicks.push_back(
		    WebSocketRuntimeState::ActiveMatchTick{match, now + state.tickInterval, now + state.snapshotInterval});
	}

	void advanceDueWebSocketMatches(WebSocketRuntimeState& state, std::chrono::steady_clock::time_point now)
	{
		bool ticked = false;
		for (auto& active : state.activeMatchTicks)
		{
			std::size_t catchUpTicks = 0;
			while (now >= active.nextTickAt && catchUpTicks < 4)
			{
				const bool broadcastSnapshot = active.nextTickAt >= active.nextSnapshotAt;
				const auto result = state.matches.tick(active.match, broadcastSnapshot);
				ticked = ticked || result.accepted;
				if (broadcastSnapshot)
				{
					do
					{
						active.nextSnapshotAt += state.snapshotInterval;
					} while (active.nextSnapshotAt <= active.nextTickAt);
				}
				active.nextTickAt += state.tickInterval;
				++catchUpTicks;
			}
			if (now >= active.nextTickAt)
			{
				active.nextTickAt = now + state.tickInterval;
				if (now >= active.nextSnapshotAt)
				{
					active.nextSnapshotAt = now + state.snapshotInterval;
				}
			}
		}
		if (ticked)
		{
			flushAllSessions(state);
		}
	}

	void handleWebSocketClient(WebSocketConnection connection, WebSocketRuntimeState& state)
	{
		WebSocketBackendOutbound outbound{connection};
		WebSocketSessionAdapter adapter;
		SessionId sessionId{};
		{
			std::lock_guard lock(state.mutex);
			const auto created = state.sessions.createSession(ConnectionId{state.nextConnectionId++}, outbound);
			if (!created.accepted() || !created.session.has_value())
			{
				sendError(connection, "capacity_reached", "server session capacity reached");
				return;
			}
			sessionId = *created.session;
			state.liveSessions.push_back(sessionId);
		}

		std::optional<MatchId> currentMatch;
		while (connection.valid())
		{
			auto read = connection.readText();
			if (read.status == WebSocketReadStatus::TimedOut)
			{
				const auto now = WebSocketSessionAdapter::Clock::now();
				{
					std::lock_guard lock(state.mutex);
					advanceDueWebSocketMatches(state, now);
				}
				const auto timeout = adapter.checkTimeout(now);
				if (timeout.action == if_arena::battle_transport_ws::WebSocketLifecycleAction::SendPing)
				{
					sendServerEnvelope(connection, MessageType::Ping, "{}");
				}
				if (timeout.action == if_arena::battle_transport_ws::WebSocketLifecycleAction::Close)
				{
					std::cerr << "WebSocket session closing session=" << sessionId.value << " reason="
					          << (timeout.error.has_value() ? timeout.error->message : "timeout") << '\n';
					sendError(connection, "timeout", timeout.error.has_value() ? timeout.error->message : "timeout");
					break;
				}
				continue;
			}
			if (read.status == WebSocketReadStatus::Closed)
			{
				break;
			}
			if (read.status == WebSocketReadStatus::Error)
			{
				break;
			}

			auto received = adapter.receiveText(read.text);
			if (!received.ok())
			{
				std::cerr << "WebSocket session protocol error session=" << sessionId.value << " reason="
				          << (received.error.has_value() ? received.error->message : "invalid message") << '\n';
				sendError(connection, "protocol_error", received.error.has_value() ? received.error->message : "invalid message");
				break;
			}

			if (received.envelope->type == MessageType::AuthRequest)
			{
				if (!validateWebSocketAuth(state.config, *received.envelope))
				{
					sendError(connection, "auth_failed", "authentication failed");
					break;
				}
				std::lock_guard lock(state.mutex);
				auto* session = state.sessions.find(sessionId);
				if (session == nullptr || !session->authenticate(PlayerId{sessionId.value}).accepted)
				{
					sendError(connection, "auth_failed", "backend authentication failed");
					break;
				}
				adapter.markAuthenticated();
				sendServerEnvelope(connection, MessageType::AuthResult,
				                   "{\"accepted\":true,\"sessionId\":\"" + std::to_string(sessionId.value) + "\"}");
				continue;
			}

			if (received.envelope->type == MessageType::CreateMatch)
			{
				std::lock_guard lock(state.mutex);
				const auto created = state.matches.createMatch(sessionId);
				if (!created.accepted() || !created.match.has_value())
				{
					sendError(connection, "create_match_rejected", backendReasonName(created.result.reason));
					continue;
				}
				currentMatch = *created.match;
				adapter.markInMatch();
				sendServerEnvelope(connection, MessageType::MatchJoined,
				                   "{\"matchId\":\"" + std::to_string(created.match->value) + "\",\"matchCode\":\"" +
				                    created.joinCode + "\",\"team\":\"blue\"}");
				continue;
			}

			if (received.envelope->type == MessageType::JoinMatch)
			{
				const auto joinCode = stringField(received.envelope->payloadJson, "matchCode");
				if (!joinCode.has_value())
				{
					sendError(connection, "join_match_rejected", "missing match code");
					continue;
				}
				std::lock_guard lock(state.mutex);
				const auto joined = state.matches.joinMatch(sessionId, *joinCode);
				if (!joined.accepted() || !joined.match.has_value())
				{
					sendError(connection, "join_match_rejected", backendReasonName(joined.result.reason));
					continue;
				}
				currentMatch = *joined.match;
				adapter.markInMatch();
				sendServerEnvelope(connection, MessageType::MatchJoined,
				                   "{\"matchId\":\"" + std::to_string(joined.match->value) + "\",\"matchCode\":\"" +
				                    *joinCode + "\",\"team\":\"red\"}");
				activateWebSocketMatchTicker(state, *currentMatch, WebSocketSessionAdapter::Clock::now());
				state.matches.tick(*currentMatch);
				flushAllSessions(state);
				continue;
			}

			if (received.envelope->type == MessageType::InputCommand)
			{
				MatchId match{};
				const auto command = parseBackendCommand(*received.envelope, match);
				if (!command.has_value() || !received.envelope->sessionSeq.has_value())
				{
					sendError(connection, "input_rejected", "invalid input command payload");
					continue;
				}
				std::lock_guard lock(state.mutex);
				const auto submitted = state.matches.submitCommand(sessionId, match, *received.envelope->sessionSeq, *command);
				sendServerEnvelope(connection, MessageType::InputAck,
				                   "{\"accepted\":" + std::string{submitted.accepted ? "true" : "false"} +
				                    ",\"reason\":\"" + backendReasonName(submitted.reason) + "\"}");
				if (submitted.accepted)
				{
					advanceDueWebSocketMatches(state, WebSocketSessionAdapter::Clock::now());
				}
				continue;
			}

			if (received.envelope->type == MessageType::Ping)
			{
				sendServerEnvelope(connection, MessageType::Pong, "{}");
				continue;
			}
			if (received.envelope->type == MessageType::Pong)
			{
				adapter.markPongReceived();
			}
		}

		{
			std::lock_guard lock(state.mutex);
			state.matches.disconnect(sessionId, DisconnectReason::ClientClosed);
		}
	}

	int runTcpServer(const ServerConfig& config, const BackendLimits& limits, SessionRegistry& sessions,
	                 MatchManager& matches, std::size_t maxClients)
	{
		const auto tickIntervalMs = std::max<std::uint32_t>(1u, 1000u / config.tickRate);
		const auto snapshotIntervalMs = std::max<std::uint32_t>(1u, 1000u / config.snapshotRate);
		const auto socketPollTimeoutMs = std::min({config.handshakeTimeoutMs, config.idleTimeoutMs, tickIntervalMs});
		TcpEndpoint endpoint;
		endpoint.host = config.tcp.host;
		endpoint.port = static_cast<std::uint16_t>(config.tcp.port);
		endpoint.frameLimits.maxFrameBytes = static_cast<std::uint32_t>(config.tcp.maxBytes);
		endpoint.frameLimits.maxBufferedBytes = config.tcp.maxBytes * 2u;
		endpoint.receiveTimeoutMs = socketPollTimeoutMs;
		endpoint.sendTimeoutMs = config.idleTimeoutMs;

		std::optional<if_arena::battle_transport_tcp::TcpSocketError> error;
		auto listener = TcpListener::bindAndListen(endpoint, error);
		if (!listener.has_value())
		{
			std::cerr << "error: failed to start TCP listener: " << (error.has_value() ? error->message : "unknown error") << '\n';
			return 1;
		}

		std::cout << "TCP listener started host=" << config.tcp.host << " port=" << config.tcp.port
		          << " maxFrameBytes=" << config.tcp.maxBytes << '\n';
		TcpRuntimeState state{sessions, matches, limits};
		state.tickInterval = std::chrono::milliseconds{tickIntervalMs};
		state.snapshotInterval = std::chrono::milliseconds{snapshotIntervalMs};
		state.handshakeTimeout = std::chrono::milliseconds{config.handshakeTimeoutMs};
		state.idleTimeout = std::chrono::milliseconds{config.idleTimeoutMs};
		std::vector<std::thread> workers;
		std::size_t accepted = 0;
		while (maxClients == 0 || accepted < maxClients)
		{
			std::optional<if_arena::battle_transport_tcp::TcpSocketError> acceptError;
			auto client = listener->accept(acceptError);
			if (!client.has_value())
			{
				if (acceptError.has_value() &&
				    acceptError->code == if_arena::battle_transport_tcp::TcpSocketErrorCode::TimedOut)
				{
					continue;
				}
				std::cerr << "error: TCP accept failed: "
				          << (acceptError.has_value() ? acceptError->message : "unknown error") << '\n';
				return 1;
			}
			++accepted;
			workers.emplace_back([connection = std::move(*client), &state]() mutable {
				handleTcpClient(std::move(connection), state);
			});
		}

		for (auto& worker : workers)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
		std::cout << "TCP listener stopped acceptedClients=" << accepted << '\n';
		return 0;
	}

	int runWebSocketServer(const ServerConfig& config, const BackendLimits& limits, SessionRegistry& sessions,
	                       MatchManager& matches, std::size_t maxClients)
	{
		const auto tickIntervalMs = std::max<std::uint32_t>(1u, 1000u / config.tickRate);
		const auto snapshotIntervalMs = std::max<std::uint32_t>(1u, 1000u / config.snapshotRate);
		const auto socketPollTimeoutMs = std::min({config.handshakeTimeoutMs, config.idleTimeoutMs, tickIntervalMs});
		WebSocketEndpoint endpoint;
		endpoint.host = config.websocket.host;
		endpoint.port = static_cast<std::uint16_t>(config.websocket.port);
		endpoint.path = config.websocket.path;
		endpoint.limits.maxMessageBytes = config.websocket.maxBytes;
		endpoint.limits.maxPendingBytes = config.maxPendingWriteBytesPerSession;
		endpoint.limits.maxPendingMessages = config.maxPendingOutboundMessages;
		endpoint.limits.handshakeTimeout = std::chrono::milliseconds{config.handshakeTimeoutMs};
		endpoint.limits.idleTimeout = std::chrono::milliseconds{config.idleTimeoutMs};
		endpoint.receiveTimeoutMs = socketPollTimeoutMs;
		endpoint.sendTimeoutMs = config.idleTimeoutMs;

		std::optional<if_arena::battle_transport_ws::WebSocketSocketError> error;
		auto listener = WebSocketListener::bindAndListen(endpoint, error);
		if (!listener.has_value())
		{
			std::cerr << "error: failed to start WebSocket listener: "
			          << (error.has_value() ? error->message : "unknown error") << '\n';
			return 1;
		}

		std::cout << "WebSocket listener started host=" << config.websocket.host << " port=" << config.websocket.port
		          << " path=" << config.websocket.path << " maxMessageBytes=" << config.websocket.maxBytes << '\n';
		WebSocketRuntimeState state{sessions, matches, limits, config};
		state.tickInterval = std::chrono::milliseconds{tickIntervalMs};
		state.snapshotInterval = std::chrono::milliseconds{snapshotIntervalMs};
		std::vector<std::thread> workers;
		std::size_t accepted = 0;
		while (maxClients == 0 || accepted < maxClients)
		{
			std::optional<if_arena::battle_transport_ws::WebSocketSocketError> acceptError;
			auto client = listener->accept(acceptError);
			if (!client.has_value())
			{
				if (acceptError.has_value() &&
				    acceptError->code == if_arena::battle_transport_ws::WebSocketSocketErrorCode::TimedOut)
				{
					continue;
				}
				std::cerr << "error: WebSocket accept failed: "
				          << (acceptError.has_value() ? acceptError->message : "unknown error") << '\n';
				return 1;
			}
			++accepted;
			workers.emplace_back([connection = std::move(*client), &state]() mutable {
				handleWebSocketClient(std::move(connection), state);
			});
		}

		for (auto& worker : workers)
		{
			if (worker.joinable())
			{
				worker.join();
			}
		}
		std::cout << "WebSocket listener stopped acceptedClients=" << accepted << '\n';
		return 0;
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
	if (config.tcp.enabled && config.websocket.enabled)
	{
		std::cout << "Both TCP and WebSocket are enabled; starting TCP listener for compatibility. Disable TCP in config to run the local WebSocket listener.\n";
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

	const int result = config.tcp.enabled ? runTcpServer(config, limits, sessions, matches, options.maxClients)
	                                     : runWebSocketServer(config, limits, sessions, matches, options.maxClients);
	std::cout << "Shutdown complete.\n";
	return result;
}
