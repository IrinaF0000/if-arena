#include "Protocol.hpp"

#include <array>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <cerrno>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

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
		bool fakeConnect{};
		bool createMatch{true};
		bool interactive{};
		bool help{};
	};

	struct ScriptCommand
	{
		std::string kind;
		int dx{};
		int dy{};
		std::uint32_t waitMs{};
		bool hasDirection{};
	};

	std::string jsonString(std::string_view value);
	void addError(std::vector<std::string>& errors, std::string message);

#ifdef _WIN32
	using NativeSocket = SOCKET;
	constexpr NativeSocket InvalidSocket = INVALID_SOCKET;
#else
	using NativeSocket = int;
	constexpr NativeSocket InvalidSocket = -1;
#endif

	void closeSocket(NativeSocket socket)
	{
		if (socket == InvalidSocket)
		{
			return;
		}
#ifdef _WIN32
		closesocket(socket);
#else
		::close(socket);
#endif
	}

	struct SocketHandle
	{
		NativeSocket socket{InvalidSocket};

		SocketHandle() = default;
		explicit SocketHandle(NativeSocket value)
			: socket(value)
		{
		}
		SocketHandle(const SocketHandle&) = delete;
		SocketHandle& operator=(const SocketHandle&) = delete;
		SocketHandle(SocketHandle&& other) noexcept
			: socket(other.socket)
		{
			other.socket = InvalidSocket;
		}
		SocketHandle& operator=(SocketHandle&& other) noexcept
		{
			if (this != &other)
			{
				closeSocket(socket);
				socket = other.socket;
				other.socket = InvalidSocket;
			}
			return *this;
		}
		~SocketHandle()
		{
			closeSocket(socket);
		}

		[[nodiscard]] bool valid() const
		{
			return socket != InvalidSocket;
		}
	};

	bool ensureSocketRuntime(std::vector<std::string>& errors)
	{
#ifdef _WIN32
		static bool started = [] {
			WSADATA data{};
			return WSAStartup(MAKEWORD(2, 2), &data) == 0;
		}();
		if (!started)
		{
			addError(errors, "WSAStartup failed");
			return false;
		}
#else
		(void)errors;
#endif
		return true;
	}

	void printHelp()
	{
		std::cout << "Usage: battle_cli_client [--fake-connect] [--endpoint HOST:PORT] [--auth-mode demo]\n"
		          << "                         [--create | --join CODE] [--match-id ID]\n"
		          << "                         [--script PATH] [--interactive]\n"
		          << "\n"
		          << "By default the CLI opens raw TCP. Use --fake-connect to print validated intentions without a socket.\n";
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

	std::pair<std::string, std::uint16_t> parseEndpoint(const std::string& endpoint, std::vector<std::string>& errors)
	{
		const auto colon = endpoint.rfind(':');
		if (colon == std::string::npos || colon == 0 || colon + 1 >= endpoint.size())
		{
			addError(errors, "endpoint must be HOST:PORT");
			return {"127.0.0.1", 5555};
		}
		int port = 0;
		if (!parseInt(std::string_view{endpoint}.substr(colon + 1), port) || port <= 0 || port > 65535)
		{
			addError(errors, "endpoint port must be in range 1..65535");
			return {endpoint.substr(0, colon), 5555};
		}
		return {endpoint.substr(0, colon), static_cast<std::uint16_t>(port)};
	}

	bool setTimeouts(NativeSocket socket, std::uint32_t timeoutMs)
	{
#ifdef _WIN32
		const DWORD timeout = timeoutMs;
#else
		const timeval timeout{
			static_cast<time_t>(timeoutMs / 1000u),
			static_cast<suseconds_t>((timeoutMs % 1000u) * 1000u),
		};
#endif
		return setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0 &&
		       setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) == 0;
	}

	std::optional<SocketHandle> connectTcp(const std::string& endpoint, std::vector<std::string>& errors)
	{
		if (!ensureSocketRuntime(errors))
		{
			return std::nullopt;
		}
		const auto [host, port] = parseEndpoint(endpoint, errors);
		if (!errors.empty())
		{
			return std::nullopt;
		}

		addrinfo hints{};
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		addrinfo* resolved = nullptr;
		const auto portText = std::to_string(port);
		if (getaddrinfo(host.c_str(), portText.c_str(), &hints, &resolved) != 0)
		{
			addError(errors, "failed to resolve endpoint");
			return std::nullopt;
		}

		for (addrinfo* candidate = resolved; candidate != nullptr; candidate = candidate->ai_next)
		{
			NativeSocket socket = ::socket(candidate->ai_family, candidate->ai_socktype, candidate->ai_protocol);
			if (socket == InvalidSocket)
			{
				continue;
			}
			if (!setTimeouts(socket, 5000))
			{
				closeSocket(socket);
				continue;
			}
			if (::connect(socket, candidate->ai_addr, static_cast<int>(candidate->ai_addrlen)) == 0)
			{
				freeaddrinfo(resolved);
				return SocketHandle{socket};
			}
			closeSocket(socket);
		}

		freeaddrinfo(resolved);
		addError(errors, "failed to connect endpoint");
		return std::nullopt;
	}

	void appendUint32Be(std::vector<std::uint8_t>& output, std::uint32_t value)
	{
		output.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
		output.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
		output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
		output.push_back(static_cast<std::uint8_t>(value & 0xffu));
	}

	bool isSocketTimeout()
	{
#ifdef _WIN32
		const int error = WSAGetLastError();
		return error == WSAETIMEDOUT || error == WSAEWOULDBLOCK;
#else
		return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
	}

	bool sendFrame(SocketHandle& handle, std::string_view payload, std::vector<std::string>& errors)
	{
		if (payload.empty() || payload.size() > 64u * 1024u)
		{
			addError(errors, "outbound frame size is invalid");
			return false;
		}
		std::vector<std::uint8_t> bytes;
		bytes.reserve(payload.size() + 4u);
		appendUint32Be(bytes, static_cast<std::uint32_t>(payload.size()));
		for (const char ch : payload)
		{
			bytes.push_back(static_cast<std::uint8_t>(ch));
		}

		std::size_t sentBytes = 0;
		while (sentBytes < bytes.size())
		{
			const int sent = send(handle.socket, reinterpret_cast<const char*>(bytes.data() + sentBytes),
			                      static_cast<int>(bytes.size() - sentBytes), 0);
			if (sent <= 0)
			{
				addError(errors, "failed to send frame");
				return false;
			}
			sentBytes += static_cast<std::size_t>(sent);
		}
		return true;
	}

	std::optional<std::string> readFrame(SocketHandle& handle, bool& timedOut, std::vector<std::string>& errors)
	{
		timedOut = false;
		std::array<std::uint8_t, 4> header{};
		std::size_t readBytes = 0;
		while (readBytes < header.size())
		{
			const int received = recv(handle.socket, reinterpret_cast<char*>(header.data() + readBytes),
			                          static_cast<int>(header.size() - readBytes), 0);
			if (received == 0)
			{
				addError(errors, "connection closed");
				return std::nullopt;
			}
			if (received < 0)
			{
				if (isSocketTimeout())
				{
					timedOut = true;
					return std::nullopt;
				}
				addError(errors, "failed to receive frame header");
				return std::nullopt;
			}
			readBytes += static_cast<std::size_t>(received);
		}
		const std::uint32_t length = (static_cast<std::uint32_t>(header[0]) << 24u) |
		                             (static_cast<std::uint32_t>(header[1]) << 16u) |
		                             (static_cast<std::uint32_t>(header[2]) << 8u) |
		                             static_cast<std::uint32_t>(header[3]);
		if (length == 0 || length > 64u * 1024u)
		{
			addError(errors, "inbound frame length is invalid");
			return std::nullopt;
		}
		std::string payload(length, '\0');
		readBytes = 0;
		while (readBytes < payload.size())
		{
			const int received = recv(handle.socket, payload.data() + readBytes,
			                          static_cast<int>(payload.size() - readBytes), 0);
			if (received == 0)
			{
				addError(errors, "connection closed");
				return std::nullopt;
			}
			if (received < 0)
			{
				if (isSocketTimeout())
				{
					timedOut = true;
					return std::nullopt;
				}
				addError(errors, "failed to receive frame payload");
				return std::nullopt;
			}
			readBytes += static_cast<std::size_t>(received);
		}
		return payload;
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
		if (command.kind == "wait")
		{
			if (words.size() != 2)
			{
				error = "wait requires: wait MS";
				return std::nullopt;
			}
			int waitMs = 0;
			if (!parseInt(words[1], waitMs) || waitMs < 0 || waitMs > 60000)
			{
				error = "wait MS must be in range 0..60000";
				return std::nullopt;
			}
			command.waitMs = static_cast<std::uint32_t>(waitMs);
			return command;
		}
		error = "unknown command kind: " + command.kind;
		return std::nullopt;
	}

	std::vector<ScriptCommand> defaultScenario()
	{
		return {
			{"move", -1, 0, 0, true}, {"move", 0, -1, 0, true}, {"move", 0, -1, 0, true},
			{"move", 0, -1, 0, true}, {"move", 0, -1, 0, true}, {"move", 1, 0, 0, true},
			{"stop", 0, 0, 0, false}, {"interact", 0, 0, 0, false}, {"move", -1, 0, 0, true},
			{"move", -1, 0, 0, true}, {"move", 0, 1, 0, true}, {"move", 0, 1, 0, true},
			{"move", 0, 1, 0, true}, {"move", 0, 1, 0, true}, {"move", 0, 1, 0, true},
			{"move", 0, 1, 0, true}, {"move", 1, 0, 0, true},
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

	std::optional<std::string> simpleStringField(std::string_view object, std::string_view key)
	{
		const std::string needle = "\"" + std::string{key} + "\":\"";
		const auto start = object.find(needle);
		if (start == std::string_view::npos)
		{
			return std::nullopt;
		}
		const auto valueStart = start + needle.size();
		std::string value;
		bool escaped = false;
		for (std::size_t index = valueStart; index < object.size(); ++index)
		{
			const char ch = object[index];
			if (escaped)
			{
				value.push_back(ch);
				escaped = false;
				continue;
			}
			if (ch == '\\')
			{
				escaped = true;
				continue;
			}
			if (ch == '"')
			{
				return value;
			}
			value.push_back(ch);
		}
		return std::nullopt;
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

	std::optional<Envelope> receiveEnvelope(SocketHandle& socket, std::vector<std::string>& errors)
	{
		bool timedOut = false;
		auto frame = readFrame(socket, timedOut, errors);
		if (!frame.has_value())
		{
			if (timedOut)
			{
				return std::nullopt;
			}
			return std::nullopt;
		}
		auto parsed = if_arena::battle_protocol::parseEnvelope(*frame);
		if (!parsed.ok())
		{
			std::cout << "[recv] " << *frame << '\n';
			addError(errors, "received invalid protocol envelope");
			return std::nullopt;
		}
		std::cout << "[recv] " << *frame << '\n';
		return parsed.envelope;
	}

	void drainReadable(SocketHandle& socket, std::vector<std::string>& errors)
	{
		for (int index = 0; index < 4; ++index)
		{
			const auto before = errors.size();
			auto envelope = receiveEnvelope(socket, errors);
			if (!envelope.has_value())
			{
				if (errors.size() != before)
				{
					return;
				}
				return;
			}
			if (envelope->type == MessageType::Snapshot || envelope->type == MessageType::Error)
			{
				return;
			}
		}
	}

	bool sendSerialized(SocketHandle& socket, const std::string& label, const std::string& json,
	                    std::vector<std::string>& errors)
	{
		std::cout << "[send] " << label << ' ' << json << '\n';
		return sendFrame(socket, json, errors);
	}

	int runTcpClient(const CliOptions& options, const std::vector<ScriptCommand>& script)
	{
		std::vector<std::string> errors;
		auto socket = connectTcp(options.endpoint, errors);
		if (!socket.has_value())
		{
			return failWithErrors(errors);
		}
		std::cout << "[tcp] connected endpoint=" << options.endpoint << '\n';

		const auto auth = serializeValidated(
			envelope(MessageType::AuthRequest,
			         "{\"mode\":" + jsonString(options.authMode) + ",\"displayName\":" + jsonString(options.displayName) + "}"),
			ClientSessionPhase::Connected, errors);
		if (!auth.has_value() || !sendSerialized(*socket, "auth_request", *auth, errors))
		{
			return failWithErrors(errors);
		}
		receiveEnvelope(*socket, errors);
		if (!errors.empty())
		{
			return failWithErrors(errors);
		}

		const auto matchEnvelope = options.createMatch
			? envelope(MessageType::CreateMatch, "{\"mode\":\"objective_run\",\"scenario\":\"arena_small_objective_run\"}")
			: envelope(MessageType::JoinMatch, "{\"matchCode\":" + jsonString(options.joinCode) + "}");
		const auto matchJson = serializeValidated(matchEnvelope, ClientSessionPhase::Authenticated, errors);
		if (!matchJson.has_value() ||
		    !sendSerialized(*socket, options.createMatch ? "create_match" : "join_match", *matchJson, errors))
		{
			return failWithErrors(errors);
		}

		std::string matchId = options.matchId;
		std::string matchCode = options.joinCode;
		while (true)
		{
			auto received = receiveEnvelope(*socket, errors);
			if (!errors.empty())
			{
				return failWithErrors(errors);
			}
			if (!received.has_value())
			{
				continue;
			}
			if (received->type == MessageType::MatchJoined)
			{
				if (auto value = simpleStringField(received->payloadJson, "matchId"); value.has_value())
				{
					matchId = *value;
				}
				if (auto value = simpleStringField(received->payloadJson, "matchCode"); value.has_value())
				{
					matchCode = *value;
				}
				std::cout << "[state] match_joined matchId=" << matchId << " code=" << matchCode << '\n';
				break;
			}
			if (received->type == MessageType::Error)
			{
				addError(errors, "server returned error during match setup");
				return failWithErrors(errors);
			}
		}

		std::cout << "[state] waiting_for_authoritative_snapshot\n";
		while (true)
		{
			const auto before = errors.size();
			auto received = receiveEnvelope(*socket, errors);
			if (!received.has_value() && errors.size() == before)
			{
				continue;
			}
			if (!errors.empty())
			{
				return failWithErrors(errors);
			}
			if (received.has_value() && received->type == MessageType::Snapshot)
			{
				break;
			}
		}

		std::uint64_t seq = 1;
		CliOptions commandOptions = options;
		commandOptions.matchId = matchId;
		for (const auto& command : script)
		{
			if (command.kind == "wait")
			{
				std::cout << "[wait] " << command.waitMs << "ms\n";
				std::this_thread::sleep_for(std::chrono::milliseconds(command.waitMs));
				continue;
			}
			const auto json = serializeValidated(envelope(MessageType::InputCommand, commandPayload(commandOptions, command), seq),
			                                     ClientSessionPhase::InMatch, errors);
			if (!json.has_value() || !sendSerialized(*socket, "input_command", *json, errors))
			{
				return failWithErrors(errors);
			}
			drainReadable(*socket, errors);
			if (!errors.empty())
			{
				return failWithErrors(errors);
			}
			++seq;
		}

		std::cout << "[event] tcp_script_complete commands=" << script.size() << '\n';
		return 0;
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

	auto script = loadScript(options.scriptPath, errors);
	if (!script.has_value() || !errors.empty())
	{
		return failWithErrors(errors);
	}

	if (!options.fakeConnect)
	{
		return runTcpClient(options, *script);
	}

	std::cout << "[fake-connect] endpoint=" << options.endpoint
	          << " transport=not-opened reason=fake-connect requested\n";

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
		if (command.kind == "wait")
		{
			std::cout << "[wait] " << command.waitMs << "ms\n";
			continue;
		}
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
