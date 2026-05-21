#include "WebSocketSession.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>

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

namespace if_arena::battle_transport_ws
{
	namespace
	{
#ifdef _WIN32
		using NativeSocket = SOCKET;
		constexpr NativeSocket InvalidSocket = INVALID_SOCKET;
#else
		using NativeSocket = int;
		constexpr NativeSocket InvalidSocket = -1;
#endif

		constexpr std::string_view WebSocketGuid{"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"};

		WebSocketError makeError(WebSocketErrorCode code, std::string message)
		{
			return WebSocketError{code, std::move(message)};
		}

		WebSocketSocketError makeSocketError(WebSocketSocketErrorCode code, std::string message)
		{
			return WebSocketSocketError{code, std::move(message)};
		}

		WebSocketMessageResult messageError(WebSocketErrorCode code, std::string message)
		{
			return WebSocketMessageResult{std::nullopt, makeError(code, std::move(message))};
		}

		NativeSocket toNative(std::intptr_t socket)
		{
			return static_cast<NativeSocket>(socket);
		}

		std::intptr_t fromNative(NativeSocket socket)
		{
			return static_cast<std::intptr_t>(socket);
		}

		bool socketIsValid(std::intptr_t socket)
		{
			return toNative(socket) != InvalidSocket;
		}

		void closeNative(NativeSocket socket)
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

		std::string lastSocketErrorMessage()
		{
#ifdef _WIN32
			return "socket error " + std::to_string(WSAGetLastError());
#else
			return std::strerror(errno);
#endif
		}

		bool isTimeoutError()
		{
#ifdef _WIN32
			const int error = WSAGetLastError();
			return error == WSAETIMEDOUT || error == WSAEWOULDBLOCK;
#else
			return errno == EAGAIN || errno == EWOULDBLOCK;
#endif
		}

		class SocketRuntime
		{
		public:
			static bool ensureStarted(std::optional<WebSocketSocketError>& error)
			{
#ifdef _WIN32
				static SocketRuntime runtime;
				if (!runtime._started)
				{
					error = makeSocketError(WebSocketSocketErrorCode::StartupFailed, "WSAStartup failed");
					return false;
				}
#else
				(void)error;
#endif
				return true;
			}

		private:
#ifdef _WIN32
			SocketRuntime()
			{
				WSADATA data{};
				_started = WSAStartup(MAKEWORD(2, 2), &data) == 0;
			}

			~SocketRuntime()
			{
				if (_started)
				{
					WSACleanup();
				}
			}

			bool _started{};
#endif
		};

		bool setTimeouts(NativeSocket socket, std::uint32_t receiveTimeoutMs, std::uint32_t sendTimeoutMs,
		                 std::optional<WebSocketSocketError>& error)
		{
#ifdef _WIN32
			const DWORD receiveTimeout = receiveTimeoutMs;
			const DWORD sendTimeout = sendTimeoutMs;
#else
			const timeval receiveTimeout{
				static_cast<time_t>(receiveTimeoutMs / 1000u),
				static_cast<suseconds_t>((receiveTimeoutMs % 1000u) * 1000u),
			};
			const timeval sendTimeout{
				static_cast<time_t>(sendTimeoutMs / 1000u),
				static_cast<suseconds_t>((sendTimeoutMs % 1000u) * 1000u),
			};
#endif
			if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&receiveTimeout),
			               sizeof(receiveTimeout)) != 0)
			{
				error = makeSocketError(WebSocketSocketErrorCode::SocketFailed, "failed to set receive timeout");
				return false;
			}
			if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&sendTimeout),
			               sizeof(sendTimeout)) != 0)
			{
				error = makeSocketError(WebSocketSocketErrorCode::SocketFailed, "failed to set send timeout");
				return false;
			}
			return true;
		}

		addrinfo hintsFor(bool passive)
		{
			addrinfo hints{};
			hints.ai_family = AF_UNSPEC;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_protocol = IPPROTO_TCP;
			if (passive)
			{
				hints.ai_flags = AI_PASSIVE;
			}
			return hints;
		}

		std::optional<WebSocketSocketError> resolveEndpoint(const WebSocketEndpoint& endpoint, bool passive, addrinfo** output)
		{
			auto hints = hintsFor(passive);
			const auto port = std::to_string(endpoint.port);
			const char* host = endpoint.host.empty() || endpoint.host == "0.0.0.0" ? nullptr : endpoint.host.c_str();
			const int rc = getaddrinfo(host, port.c_str(), &hints, output);
			if (rc != 0)
			{
				return makeSocketError(WebSocketSocketErrorCode::ResolveFailed, "failed to resolve WebSocket endpoint");
			}
			return std::nullopt;
		}

		std::string trimCopy(std::string_view value)
		{
			std::size_t begin = 0;
			while (begin < value.size() && std::isspace(static_cast<unsigned char>(value[begin])) != 0)
			{
				++begin;
			}
			std::size_t end = value.size();
			while (end > begin && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0)
			{
				--end;
			}
			return std::string{value.substr(begin, end - begin)};
		}

		std::string lowercase(std::string value)
		{
			std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
				return static_cast<char>(std::tolower(ch));
			});
			return value;
		}

		std::uint32_t rotateLeft(std::uint32_t value, std::uint32_t bits)
		{
			return (value << bits) | (value >> (32u - bits));
		}

		std::array<std::uint8_t, 20> sha1(std::string_view input)
		{
			std::vector<std::uint8_t> message(input.begin(), input.end());
			const std::uint64_t bitLength = static_cast<std::uint64_t>(message.size()) * 8u;
			message.push_back(0x80u);
			while ((message.size() % 64u) != 56u)
			{
				message.push_back(0u);
			}
			for (int shift = 56; shift >= 0; shift -= 8)
			{
				message.push_back(static_cast<std::uint8_t>((bitLength >> static_cast<unsigned int>(shift)) & 0xffu));
			}

			std::uint32_t h0 = 0x67452301u;
			std::uint32_t h1 = 0xefcdab89u;
			std::uint32_t h2 = 0x98badcfeu;
			std::uint32_t h3 = 0x10325476u;
			std::uint32_t h4 = 0xc3d2e1f0u;

			for (std::size_t offset = 0; offset < message.size(); offset += 64u)
			{
				std::array<std::uint32_t, 80> words{};
				for (std::size_t index = 0; index < 16u; ++index)
				{
					const std::size_t base = offset + index * 4u;
					words[index] = (static_cast<std::uint32_t>(message[base]) << 24u) |
					               (static_cast<std::uint32_t>(message[base + 1u]) << 16u) |
					               (static_cast<std::uint32_t>(message[base + 2u]) << 8u) |
					               static_cast<std::uint32_t>(message[base + 3u]);
				}
				for (std::size_t index = 16u; index < words.size(); ++index)
				{
					words[index] = rotateLeft(words[index - 3u] ^ words[index - 8u] ^ words[index - 14u] ^ words[index - 16u], 1u);
				}

				std::uint32_t a = h0;
				std::uint32_t b = h1;
				std::uint32_t c = h2;
				std::uint32_t d = h3;
				std::uint32_t e = h4;
				for (std::size_t index = 0; index < words.size(); ++index)
				{
					std::uint32_t f = 0;
					std::uint32_t k = 0;
					if (index < 20u)
					{
						f = (b & c) | ((~b) & d);
						k = 0x5a827999u;
					}
					else if (index < 40u)
					{
						f = b ^ c ^ d;
						k = 0x6ed9eba1u;
					}
					else if (index < 60u)
					{
						f = (b & c) | (b & d) | (c & d);
						k = 0x8f1bbcdcu;
					}
					else
					{
						f = b ^ c ^ d;
						k = 0xca62c1d6u;
					}
					const std::uint32_t temp = rotateLeft(a, 5u) + f + e + k + words[index];
					e = d;
					d = c;
					c = rotateLeft(b, 30u);
					b = a;
					a = temp;
				}
				h0 += a;
				h1 += b;
				h2 += c;
				h3 += d;
				h4 += e;
			}

			const std::array<std::uint32_t, 5> hash{h0, h1, h2, h3, h4};
			std::array<std::uint8_t, 20> digest{};
			for (std::size_t index = 0; index < hash.size(); ++index)
			{
				digest[index * 4u] = static_cast<std::uint8_t>((hash[index] >> 24u) & 0xffu);
				digest[index * 4u + 1u] = static_cast<std::uint8_t>((hash[index] >> 16u) & 0xffu);
				digest[index * 4u + 2u] = static_cast<std::uint8_t>((hash[index] >> 8u) & 0xffu);
				digest[index * 4u + 3u] = static_cast<std::uint8_t>(hash[index] & 0xffu);
			}
			return digest;
		}

		std::string base64(std::span<const std::uint8_t> input)
		{
			constexpr char alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			std::string output;
			output.reserve(((input.size() + 2u) / 3u) * 4u);
			for (std::size_t index = 0; index < input.size(); index += 3u)
			{
				const std::uint32_t b0 = input[index];
				const std::uint32_t b1 = index + 1u < input.size() ? input[index + 1u] : 0u;
				const std::uint32_t b2 = index + 2u < input.size() ? input[index + 2u] : 0u;
				const std::uint32_t triple = (b0 << 16u) | (b1 << 8u) | b2;
				output.push_back(alphabet[(triple >> 18u) & 0x3fu]);
				output.push_back(alphabet[(triple >> 12u) & 0x3fu]);
				output.push_back(index + 1u < input.size() ? alphabet[(triple >> 6u) & 0x3fu] : '=');
				output.push_back(index + 2u < input.size() ? alphabet[triple & 0x3fu] : '=');
			}
			return output;
		}

		std::string acceptKey(std::string_view key)
		{
			std::string material{key};
			material += WebSocketGuid;
			const auto digest = sha1(material);
			return base64(digest);
		}

		std::optional<std::string> handshakeResponse(std::string_view request, const std::string& expectedPath)
		{
			std::istringstream input(std::string{request});
			std::string line;
			if (!std::getline(input, line))
			{
				return std::nullopt;
			}
			if (!line.empty() && line.back() == '\r')
			{
				line.pop_back();
			}
			std::istringstream requestLine(line);
			std::string method;
			std::string path;
			std::string version;
			requestLine >> method >> path >> version;
			if (method != "GET" || path != expectedPath || version.rfind("HTTP/", 0) != 0)
			{
				return std::nullopt;
			}

			std::map<std::string, std::string> headers;
			while (std::getline(input, line))
			{
				if (!line.empty() && line.back() == '\r')
				{
					line.pop_back();
				}
				if (line.empty())
				{
					break;
				}
				const auto colon = line.find(':');
				if (colon == std::string::npos)
				{
					continue;
				}
				headers[lowercase(trimCopy(line.substr(0, colon)))] = trimCopy(line.substr(colon + 1u));
			}

			const auto key = headers.find("sec-websocket-key");
			const auto upgrade = headers.find("upgrade");
			if (key == headers.end() || key->second.empty() || upgrade == headers.end() || lowercase(upgrade->second) != "websocket")
			{
				return std::nullopt;
			}

			std::ostringstream output;
			output << "HTTP/1.1 101 Switching Protocols\r\n"
			       << "Upgrade: websocket\r\n"
			       << "Connection: Upgrade\r\n"
			       << "Sec-WebSocket-Accept: " << acceptKey(key->second) << "\r\n\r\n";
			return output.str();
		}

		bool sendAll(NativeSocket socket, std::span<const std::uint8_t> bytes, std::optional<WebSocketSocketError>& error)
		{
			std::size_t sentBytes = 0;
			while (sentBytes < bytes.size())
			{
				const int sent = send(socket, reinterpret_cast<const char*>(bytes.data() + sentBytes),
				                      static_cast<int>(bytes.size() - sentBytes), 0);
				if (sent <= 0)
				{
					error = makeSocketError(isTimeoutError() ? WebSocketSocketErrorCode::TimedOut : WebSocketSocketErrorCode::SendFailed,
					                        isTimeoutError() ? "WebSocket send timed out" : lastSocketErrorMessage());
					return false;
				}
				sentBytes += static_cast<std::size_t>(sent);
			}
			return true;
		}
	}

	bool WebSocketMessageResult::ok() const
	{
		return envelope.has_value();
	}

	WebSocketFrameDecoder::WebSocketFrameDecoder(WebSocketLimits limits)
		: _limits(limits)
	{
	}

	WebSocketReadResult WebSocketFrameDecoder::feed(std::span<const std::uint8_t> bytes)
	{
		if (_closed)
		{
			return WebSocketReadResult{WebSocketReadStatus::Closed, {}, makeSocketError(WebSocketSocketErrorCode::Closed, "decoder is closed")};
		}
		_buffer.insert(_buffer.end(), bytes.begin(), bytes.end());
		while (true)
		{
			auto decoded = tryDecode();
			if (!decoded.has_value())
			{
				return WebSocketReadResult{WebSocketReadStatus::TimedOut, {}, std::nullopt};
			}
			if (decoded->status == WebSocketReadStatus::TimedOut)
			{
				continue;
			}
			return *decoded;
		}
	}

	bool WebSocketFrameDecoder::closed() const
	{
		return _closed;
	}

	std::optional<WebSocketReadResult> WebSocketFrameDecoder::tryDecode()
	{
		if (_buffer.size() < 2u)
		{
			return std::nullopt;
		}
		const std::uint8_t first = _buffer[0];
		const std::uint8_t second = _buffer[1];
		const bool fin = (first & 0x80u) != 0;
		const std::uint8_t opcode = first & 0x0fu;
		const bool masked = (second & 0x80u) != 0;
		std::uint64_t length = second & 0x7fu;
		std::size_t offset = 2u;

		if (!fin)
		{
			return fail(WebSocketSocketErrorCode::FrameError, "fragmented WebSocket frames are not supported");
		}
		if (!masked)
		{
			return fail(WebSocketSocketErrorCode::FrameError, "client WebSocket frames must be masked");
		}
		if (length == 126u)
		{
			if (_buffer.size() < offset + 2u)
			{
				return std::nullopt;
			}
			length = (static_cast<std::uint64_t>(_buffer[offset]) << 8u) | _buffer[offset + 1u];
			offset += 2u;
		}
		else if (length == 127u)
		{
			if (_buffer.size() < offset + 8u)
			{
				return std::nullopt;
			}
			length = 0;
			for (std::size_t index = 0; index < 8u; ++index)
			{
				length = (length << 8u) | _buffer[offset + index];
			}
			offset += 8u;
		}
		if (length > _limits.maxMessageBytes)
		{
			return fail(WebSocketSocketErrorCode::FrameError, "WebSocket message exceeds limit");
		}
		if (_buffer.size() < offset + 4u + static_cast<std::size_t>(length))
		{
			return std::nullopt;
		}

		std::array<std::uint8_t, 4> mask{};
		std::copy_n(_buffer.begin() + static_cast<std::ptrdiff_t>(offset), 4, mask.begin());
		offset += 4u;

		std::string payload;
		payload.resize(static_cast<std::size_t>(length));
		for (std::size_t index = 0; index < payload.size(); ++index)
		{
			payload[index] = static_cast<char>(_buffer[offset + index] ^ mask[index % mask.size()]);
		}
		_buffer.erase(_buffer.begin(), _buffer.begin() + static_cast<std::ptrdiff_t>(offset + payload.size()));

		if (opcode == 0x8u)
		{
			_closed = true;
			return WebSocketReadResult{WebSocketReadStatus::Closed, {}, std::nullopt};
		}
		if (opcode == 0x1u)
		{
			return WebSocketReadResult{WebSocketReadStatus::Text, std::move(payload), std::nullopt};
		}
		if (opcode == 0x9u || opcode == 0xau)
		{
			return WebSocketReadResult{WebSocketReadStatus::TimedOut, {}, std::nullopt};
		}
		return fail(WebSocketSocketErrorCode::FrameError, "unsupported WebSocket opcode");
	}

	WebSocketReadResult WebSocketFrameDecoder::fail(WebSocketSocketErrorCode code, std::string message)
	{
		_closed = true;
		_buffer.clear();
		return WebSocketReadResult{WebSocketReadStatus::Error, {}, makeSocketError(code, std::move(message))};
	}

	std::vector<std::uint8_t> encodeWebSocketTextFrame(std::string_view payload)
	{
		std::vector<std::uint8_t> bytes;
		bytes.push_back(0x81u);
		if (payload.size() <= 125u)
		{
			bytes.push_back(static_cast<std::uint8_t>(payload.size()));
		}
		else if (payload.size() <= 65535u)
		{
			bytes.push_back(126u);
			bytes.push_back(static_cast<std::uint8_t>((payload.size() >> 8u) & 0xffu));
			bytes.push_back(static_cast<std::uint8_t>(payload.size() & 0xffu));
		}
		else
		{
			bytes.push_back(127u);
			const auto length = static_cast<std::uint64_t>(payload.size());
			for (int shift = 56; shift >= 0; shift -= 8)
			{
				bytes.push_back(static_cast<std::uint8_t>((length >> static_cast<unsigned int>(shift)) & 0xffu));
			}
		}
		std::transform(payload.begin(), payload.end(), std::back_inserter(bytes), [](char ch) {
			return static_cast<std::uint8_t>(ch);
		});
		return bytes;
	}

	WebSocketSessionAdapter::WebSocketSessionAdapter(WebSocketLimits limits)
		: WebSocketSessionAdapter(limits, Clock::now())
	{
	}

	WebSocketSessionAdapter::WebSocketSessionAdapter(WebSocketLimits limits, Clock::time_point now)
		: _limits(limits),
		  _connectedAt(now),
		  _lastActivityAt(now)
	{
	}

	WebSocketMessageResult WebSocketSessionAdapter::receiveText(std::string_view message, Clock::time_point now)
	{
		if (_closed)
		{
			return messageError(WebSocketErrorCode::Closed, "session is closed");
		}
		if (message.size() > _limits.maxMessageBytes)
		{
			close(battle_backend::DisconnectReason::ProtocolError);
			return messageError(WebSocketErrorCode::MessageTooLarge, "message is too large");
		}

		battle_protocol::ProtocolLimits protocolLimits;
		protocolLimits.maxMessageBytes = _limits.maxMessageBytes;
		const auto parsed = battle_protocol::parseEnvelope(message, protocolLimits);
		if (!parsed.ok())
		{
			close(battle_backend::DisconnectReason::ProtocolError);
			return messageError(WebSocketErrorCode::MalformedMessage, "invalid protocol message");
		}

		const auto validation = battle_protocol::validateClientEnvelope(*parsed.envelope, _phase, protocolLimits);
		if (validation.code != battle_protocol::ProtocolErrorCode::None)
		{
			close(battle_backend::DisconnectReason::ProtocolError);
			return messageError(WebSocketErrorCode::InvalidMessageOrder, validation.message);
		}

		_lastActivityAt = now;
		_pingOutstanding = false;
		return WebSocketMessageResult{parsed.envelope, std::nullopt};
	}

	WebSocketLifecycleResult WebSocketSessionAdapter::checkTimeout(Clock::time_point now)
	{
		if (_closed)
		{
			return WebSocketLifecycleResult{WebSocketLifecycleAction::Close,
			                                makeError(WebSocketErrorCode::Closed, "session is closed")};
		}
		if (_phase == battle_protocol::ClientSessionPhase::Connected && now - _connectedAt >= _limits.handshakeTimeout)
		{
			close(battle_backend::DisconnectReason::ProtocolError);
			return WebSocketLifecycleResult{WebSocketLifecycleAction::Close,
			                                makeError(WebSocketErrorCode::HandshakeTimeout, "handshake timed out")};
		}
		if (now - _lastActivityAt >= _limits.idleTimeout)
		{
			if (_pingOutstanding)
			{
				close(battle_backend::DisconnectReason::ClientClosed);
				return WebSocketLifecycleResult{WebSocketLifecycleAction::Close,
				                                makeError(WebSocketErrorCode::IdleTimeout, "idle timeout")};
			}
			_pingOutstanding = true;
			_lastActivityAt = now;
			return WebSocketLifecycleResult{WebSocketLifecycleAction::SendPing, std::nullopt};
		}
		return WebSocketLifecycleResult{WebSocketLifecycleAction::None, std::nullopt};
	}

	void WebSocketSessionAdapter::markAuthenticated()
	{
		if (!_closed)
		{
			_phase = battle_protocol::ClientSessionPhase::Authenticated;
		}
	}

	void WebSocketSessionAdapter::markInMatch()
	{
		if (!_closed)
		{
			_phase = battle_protocol::ClientSessionPhase::InMatch;
		}
	}

	void WebSocketSessionAdapter::markPongReceived(Clock::time_point now)
	{
		if (!_closed)
		{
			_lastActivityAt = now;
			_pingOutstanding = false;
		}
	}

	bool WebSocketSessionAdapter::send(std::string_view payload)
	{
		if (_closed || payload.size() > _limits.maxMessageBytes)
		{
			return false;
		}
		if (_sentMessages.size() >= _limits.maxPendingMessages ||
		    _pendingOutboundBytes + payload.size() > _limits.maxPendingBytes)
		{
			close(battle_backend::DisconnectReason::QueueOverflow);
			return false;
		}

		++_sentMessageCount;
		_pendingOutboundBytes += payload.size();
		_sentMessages.emplace_back(payload);
		return true;
	}

	void WebSocketSessionAdapter::close(battle_backend::DisconnectReason reason)
	{
		_closed = true;
		_closeReason = reason;
	}

	bool WebSocketSessionAdapter::closed() const
	{
		return _closed;
	}

	battle_backend::DisconnectReason WebSocketSessionAdapter::closeReason() const
	{
		return _closeReason;
	}

	std::size_t WebSocketSessionAdapter::sentMessageCount() const
	{
		return _sentMessageCount;
	}

	std::size_t WebSocketSessionAdapter::pendingOutboundBytes() const
	{
		return _pendingOutboundBytes;
	}

	battle_protocol::ClientSessionPhase WebSocketSessionAdapter::phase() const
	{
		return _phase;
	}

	const std::vector<std::string>& WebSocketSessionAdapter::sentMessages() const
	{
		return _sentMessages;
	}

	WebSocketConnection::WebSocketConnection(std::intptr_t socketHandle, WebSocketLimits limits)
		: _socket(socketHandle),
		  _limits(limits),
		  _decoder(limits)
	{
	}

	WebSocketConnection::WebSocketConnection(WebSocketConnection&& other) noexcept
		: _socket(other._socket),
		  _limits(other._limits),
		  _decoder(std::move(other._decoder)),
		  _pendingText(std::move(other._pendingText))
	{
		other._socket = fromNative(InvalidSocket);
	}

	WebSocketConnection& WebSocketConnection::operator=(WebSocketConnection&& other) noexcept
	{
		if (this != &other)
		{
			close();
			_socket = other._socket;
			_limits = other._limits;
			_decoder = std::move(other._decoder);
			_pendingText = std::move(other._pendingText);
			other._socket = fromNative(InvalidSocket);
		}
		return *this;
	}

	WebSocketConnection::~WebSocketConnection()
	{
		close();
	}

	bool WebSocketConnection::valid() const
	{
		return socketIsValid(_socket);
	}

	WebSocketReadResult WebSocketConnection::readText()
	{
		if (!_pendingText.empty())
		{
			auto text = std::move(_pendingText.front());
			_pendingText.erase(_pendingText.begin());
			return WebSocketReadResult{WebSocketReadStatus::Text, std::move(text), std::nullopt};
		}
		if (!valid())
		{
			return WebSocketReadResult{WebSocketReadStatus::Closed, {}, makeSocketError(WebSocketSocketErrorCode::Closed, "connection is closed")};
		}

		std::array<std::uint8_t, 4096> buffer{};
		while (true)
		{
			const int received = recv(toNative(_socket), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
			if (received == 0)
			{
				close();
				return WebSocketReadResult{WebSocketReadStatus::Closed, {}, std::nullopt};
			}
			if (received < 0)
			{
				if (isTimeoutError())
				{
					return WebSocketReadResult{WebSocketReadStatus::TimedOut, {}, makeSocketError(WebSocketSocketErrorCode::TimedOut, "WebSocket read timed out")};
				}
				close();
				return WebSocketReadResult{WebSocketReadStatus::Error, {}, makeSocketError(WebSocketSocketErrorCode::ReceiveFailed, lastSocketErrorMessage())};
			}

			auto decoded = _decoder.feed(std::span<const std::uint8_t>{buffer.data(), static_cast<std::size_t>(received)});
			if (decoded.status == WebSocketReadStatus::Text || decoded.status == WebSocketReadStatus::Closed ||
			    decoded.status == WebSocketReadStatus::Error)
			{
				if (decoded.status == WebSocketReadStatus::Closed || decoded.status == WebSocketReadStatus::Error)
				{
					close();
				}
				return decoded;
			}
		}
	}

	WebSocketIoResult WebSocketConnection::sendText(std::string_view payload)
	{
		if (!valid())
		{
			return WebSocketIoResult{false, makeSocketError(WebSocketSocketErrorCode::Closed, "connection is closed")};
		}
		if (payload.size() > _limits.maxMessageBytes)
		{
			return WebSocketIoResult{false, makeSocketError(WebSocketSocketErrorCode::FrameError, "payload exceeds WebSocket limit")};
		}
		const auto frame = encodeWebSocketTextFrame(payload);
		std::optional<WebSocketSocketError> error;
		if (!sendAll(toNative(_socket), frame, error))
		{
			close();
			return WebSocketIoResult{false, error};
		}
		return WebSocketIoResult{true, std::nullopt};
	}

	void WebSocketConnection::close()
	{
		if (valid())
		{
			closeNative(toNative(_socket));
			_socket = fromNative(InvalidSocket);
		}
	}

	WebSocketListener::WebSocketListener(std::intptr_t socketHandle, WebSocketEndpoint endpoint)
		: _socket(socketHandle),
		  _endpoint(std::move(endpoint))
	{
	}

	WebSocketListener::WebSocketListener(WebSocketListener&& other) noexcept
		: _socket(other._socket),
		  _endpoint(std::move(other._endpoint))
	{
		other._socket = fromNative(InvalidSocket);
	}

	WebSocketListener& WebSocketListener::operator=(WebSocketListener&& other) noexcept
	{
		if (this != &other)
		{
			close();
			_socket = other._socket;
			_endpoint = std::move(other._endpoint);
			other._socket = fromNative(InvalidSocket);
		}
		return *this;
	}

	WebSocketListener::~WebSocketListener()
	{
		close();
	}

	std::optional<WebSocketListener> WebSocketListener::bindAndListen(const WebSocketEndpoint& endpoint,
	                                                                  std::optional<WebSocketSocketError>& error)
	{
		if (!SocketRuntime::ensureStarted(error))
		{
			return std::nullopt;
		}

		addrinfo* resolved = nullptr;
		if (auto resolveError = resolveEndpoint(endpoint, true, &resolved); resolveError.has_value())
		{
			error = std::move(resolveError);
			return std::nullopt;
		}

		for (addrinfo* candidate = resolved; candidate != nullptr; candidate = candidate->ai_next)
		{
			NativeSocket socket = ::socket(candidate->ai_family, candidate->ai_socktype, candidate->ai_protocol);
			if (socket == InvalidSocket)
			{
				continue;
			}
			int reuse = 1;
			setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse));
			if (!setTimeouts(socket, endpoint.receiveTimeoutMs, endpoint.sendTimeoutMs, error))
			{
				closeNative(socket);
				continue;
			}
			if (::bind(socket, candidate->ai_addr, static_cast<int>(candidate->ai_addrlen)) != 0)
			{
				closeNative(socket);
				continue;
			}
			if (::listen(socket, 16) != 0)
			{
				error = makeSocketError(WebSocketSocketErrorCode::ListenFailed, "failed to listen on WebSocket endpoint");
				closeNative(socket);
				continue;
			}
			freeaddrinfo(resolved);
			error.reset();
			return WebSocketListener{fromNative(socket), endpoint};
		}

		freeaddrinfo(resolved);
		error = makeSocketError(WebSocketSocketErrorCode::BindFailed, "failed to bind WebSocket endpoint");
		return std::nullopt;
	}

	bool WebSocketListener::valid() const
	{
		return socketIsValid(_socket);
	}

	std::optional<WebSocketConnection> WebSocketListener::accept(std::optional<WebSocketSocketError>& error)
	{
		if (!valid())
		{
			error = makeSocketError(WebSocketSocketErrorCode::Closed, "listener is closed");
			return std::nullopt;
		}

		NativeSocket client = ::accept(toNative(_socket), nullptr, nullptr);
		if (client == InvalidSocket)
		{
			if (isTimeoutError())
			{
				error = makeSocketError(WebSocketSocketErrorCode::TimedOut, "WebSocket accept timed out");
			}
			else
			{
				error = makeSocketError(WebSocketSocketErrorCode::AcceptFailed, lastSocketErrorMessage());
			}
			return std::nullopt;
		}
		if (!setTimeouts(client, _endpoint.receiveTimeoutMs, _endpoint.sendTimeoutMs, error))
		{
			closeNative(client);
			return std::nullopt;
		}

		std::string request;
		std::array<char, 1024> buffer{};
		while (request.find("\r\n\r\n") == std::string::npos)
		{
			const int received = recv(client, buffer.data(), static_cast<int>(buffer.size()), 0);
			if (received <= 0)
			{
				error = makeSocketError(received < 0 && isTimeoutError() ? WebSocketSocketErrorCode::TimedOut
				                                                          : WebSocketSocketErrorCode::HandshakeFailed,
				                        received < 0 && isTimeoutError() ? "WebSocket handshake timed out"
				                                                          : "WebSocket handshake read failed");
				closeNative(client);
				return std::nullopt;
			}
			request.append(buffer.data(), static_cast<std::size_t>(received));
			if (request.size() > 8192u)
			{
				error = makeSocketError(WebSocketSocketErrorCode::HandshakeFailed, "WebSocket handshake is too large");
				closeNative(client);
				return std::nullopt;
			}
		}

		const auto response = handshakeResponse(request, _endpoint.path);
		if (!response.has_value())
		{
			error = makeSocketError(WebSocketSocketErrorCode::HandshakeFailed, "invalid WebSocket handshake");
			closeNative(client);
			return std::nullopt;
		}
		std::vector<std::uint8_t> bytes(response->begin(), response->end());
		if (!sendAll(client, bytes, error))
		{
			closeNative(client);
			return std::nullopt;
		}
		error.reset();
		return WebSocketConnection{fromNative(client), _endpoint.limits};
	}

	void WebSocketListener::close()
	{
		if (valid())
		{
			closeNative(toNative(_socket));
			_socket = fromNative(InvalidSocket);
		}
	}
}
