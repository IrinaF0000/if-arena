#include "TcpFrameCodec.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iterator>
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

namespace if_arena::battle_transport_tcp
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

		TcpFrameError makeError(TcpFrameErrorCode code, std::string message)
		{
			return TcpFrameError{code, std::move(message)};
		}

		TcpSocketError makeSocketError(TcpSocketErrorCode code, std::string message)
		{
			return TcpSocketError{code, std::move(message)};
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

		class SocketRuntime
		{
		public:
			static bool ensureStarted(std::optional<TcpSocketError>& error)
			{
#ifdef _WIN32
				static SocketRuntime runtime;
				if (!runtime._started)
				{
					error = makeSocketError(TcpSocketErrorCode::StartupFailed, "WSAStartup failed");
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

		void appendUint32Be(std::vector<std::uint8_t>& output, std::uint32_t value)
		{
			output.push_back(static_cast<std::uint8_t>((value >> 24u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>((value >> 16u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>((value >> 8u) & 0xffu));
			output.push_back(static_cast<std::uint8_t>(value & 0xffu));
		}

		bool setTimeouts(NativeSocket socket, std::uint32_t receiveTimeoutMs, std::uint32_t sendTimeoutMs,
		                 std::optional<TcpSocketError>& error)
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
				error = makeSocketError(TcpSocketErrorCode::SocketFailed, "failed to set receive timeout");
				return false;
			}
			if (setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&sendTimeout),
			               sizeof(sendTimeout)) != 0)
			{
				error = makeSocketError(TcpSocketErrorCode::SocketFailed, "failed to set send timeout");
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

		std::optional<TcpSocketError> resolveEndpoint(const TcpEndpoint& endpoint, bool passive, addrinfo** output)
		{
			auto hints = hintsFor(passive);
			const auto port = std::to_string(endpoint.port);
			const char* host = endpoint.host.empty() || endpoint.host == "0.0.0.0" ? nullptr : endpoint.host.c_str();
			const int rc = getaddrinfo(host, port.c_str(), &hints, output);
			if (rc != 0)
			{
				return makeSocketError(TcpSocketErrorCode::ResolveFailed, "failed to resolve TCP endpoint");
			}
			return std::nullopt;
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
	}

	bool TcpFrameDecodeResult::ok() const
	{
		return !error.has_value();
	}

	bool TcpFrameEncodeResult::ok() const
	{
		return !error.has_value();
	}

	TcpFrameDecoder::TcpFrameDecoder(TcpFrameLimits limits)
		: _limits(limits)
	{
	}

	TcpFrameDecodeResult TcpFrameDecoder::feed(std::span<const std::uint8_t> bytes)
	{
		if (_error.has_value())
		{
			return TcpFrameDecodeResult{{}, makeError(TcpFrameErrorCode::DecoderClosed, "decoder is closed")};
		}

		if (_buffer.size() + bytes.size() > _limits.maxBufferedBytes)
		{
			return TcpFrameDecodeResult{{}, fail(TcpFrameErrorCode::BufferLimitExceeded, "buffer limit exceeded")};
		}

		_buffer.insert(_buffer.end(), bytes.begin(), bytes.end());

		TcpFrameDecodeResult result;
		while (_buffer.size() >= 4u)
		{
			const std::uint32_t length = peekLength();
			if (length == 0u)
			{
				result.error = fail(TcpFrameErrorCode::InvalidLength, "frame length must be positive");
				return result;
			}
			if (length > _limits.maxFrameBytes)
			{
				result.error = fail(TcpFrameErrorCode::FrameTooLarge, "frame length exceeds limit");
				return result;
			}

			const auto frameBytes = static_cast<std::size_t>(length);
			const auto totalBytes = 4u + frameBytes;
			if (_buffer.size() < totalBytes)
			{
				break;
			}

			const auto payloadBegin = _buffer.begin() + 4;
			const auto payloadEnd = payloadBegin + static_cast<std::ptrdiff_t>(frameBytes);
			result.frames.emplace_back(payloadBegin, payloadEnd);
			_buffer.erase(_buffer.begin(), payloadEnd);
		}

		return result;
	}

	void TcpFrameDecoder::reset()
	{
		_buffer.clear();
		_error.reset();
	}

	bool TcpFrameDecoder::closed() const
	{
		return _error.has_value();
	}

	std::size_t TcpFrameDecoder::bufferedBytes() const
	{
		return _buffer.size();
	}

	std::uint32_t TcpFrameDecoder::peekLength() const
	{
		return (static_cast<std::uint32_t>(_buffer[0]) << 24u) |
			   (static_cast<std::uint32_t>(_buffer[1]) << 16u) |
			   (static_cast<std::uint32_t>(_buffer[2]) << 8u) |
			   static_cast<std::uint32_t>(_buffer[3]);
	}

	TcpFrameError TcpFrameDecoder::fail(TcpFrameErrorCode code, std::string message)
	{
		_error = makeError(code, std::move(message));
		_buffer.clear();
		return *_error;
	}

	TcpFrameEncodeResult encodeFrame(std::string_view payload, TcpFrameLimits limits)
	{
		if (payload.empty())
		{
			return TcpFrameEncodeResult{{}, makeError(TcpFrameErrorCode::InvalidLength, "payload must not be empty")};
		}
		if (payload.size() > limits.maxFrameBytes)
		{
			return TcpFrameEncodeResult{{}, makeError(TcpFrameErrorCode::FrameTooLarge, "payload exceeds frame limit")};
		}

		std::vector<std::uint8_t> bytes;
		bytes.reserve(payload.size() + 4u);
		appendUint32Be(bytes, static_cast<std::uint32_t>(payload.size()));
		std::transform(payload.begin(), payload.end(), std::back_inserter(bytes), [](char value) {
			return static_cast<std::uint8_t>(value);
		});
		return TcpFrameEncodeResult{std::move(bytes), std::nullopt};
	}

	TcpConnection::TcpConnection(std::intptr_t socketHandle, TcpFrameLimits limits)
		: _socket(socketHandle),
		  _frameLimits(limits),
		  _decoder(limits)
	{
	}

	TcpConnection::TcpConnection(TcpConnection&& other) noexcept
		: _socket(other._socket),
		  _frameLimits(other._frameLimits),
		  _decoder(std::move(other._decoder)),
		  _pendingFrames(std::move(other._pendingFrames))
	{
		other._socket = fromNative(InvalidSocket);
	}

	TcpConnection& TcpConnection::operator=(TcpConnection&& other) noexcept
	{
		if (this != &other)
		{
			close();
			_socket = other._socket;
			_frameLimits = other._frameLimits;
			_decoder = std::move(other._decoder);
			_pendingFrames = std::move(other._pendingFrames);
			other._socket = fromNative(InvalidSocket);
		}
		return *this;
	}

	TcpConnection::~TcpConnection()
	{
		close();
	}

	std::optional<TcpConnection> TcpConnection::connectTo(const TcpEndpoint& endpoint, std::optional<TcpSocketError>& error)
	{
		if (!SocketRuntime::ensureStarted(error))
		{
			return std::nullopt;
		}

		addrinfo* resolved = nullptr;
		if (auto resolveError = resolveEndpoint(endpoint, false, &resolved); resolveError.has_value())
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
			if (!setTimeouts(socket, endpoint.receiveTimeoutMs, endpoint.sendTimeoutMs, error))
			{
				closeNative(socket);
				continue;
			}
			if (::connect(socket, candidate->ai_addr, static_cast<int>(candidate->ai_addrlen)) == 0)
			{
				freeaddrinfo(resolved);
				error.reset();
				return TcpConnection{fromNative(socket), endpoint.frameLimits};
			}
			closeNative(socket);
		}

		freeaddrinfo(resolved);
		error = makeSocketError(TcpSocketErrorCode::ConnectFailed, "failed to connect TCP endpoint");
		return std::nullopt;
	}

	bool TcpConnection::valid() const
	{
		return socketIsValid(_socket);
	}

	TcpReadResult TcpConnection::readFrame()
	{
		if (!_pendingFrames.empty())
		{
			auto frame = std::move(_pendingFrames.front());
			_pendingFrames.erase(_pendingFrames.begin());
			return TcpReadResult{TcpReadStatus::Frame, std::move(frame), std::nullopt};
		}
		if (!valid())
		{
			return TcpReadResult{TcpReadStatus::Closed, {}, makeSocketError(TcpSocketErrorCode::Closed, "connection is closed")};
		}

		std::array<std::uint8_t, 4096> buffer{};
		while (true)
		{
			const int received = recv(toNative(_socket), reinterpret_cast<char*>(buffer.data()), static_cast<int>(buffer.size()), 0);
			if (received == 0)
			{
				close();
				return TcpReadResult{TcpReadStatus::Closed, {}, std::nullopt};
			}
			if (received < 0)
			{
				if (isTimeoutError())
				{
					return TcpReadResult{TcpReadStatus::TimedOut, {}, makeSocketError(TcpSocketErrorCode::TimedOut, "TCP read timed out")};
				}
				close();
				return TcpReadResult{TcpReadStatus::Error, {}, makeSocketError(TcpSocketErrorCode::ReceiveFailed, lastSocketErrorMessage())};
			}

			const auto decoded = _decoder.feed(std::span<const std::uint8_t>{buffer.data(), static_cast<std::size_t>(received)});
			if (!decoded.ok())
			{
				close();
				return TcpReadResult{TcpReadStatus::Error, {}, makeSocketError(TcpSocketErrorCode::FrameError, decoded.error->message)};
			}
			if (!decoded.frames.empty())
			{
				_pendingFrames = decoded.frames;
				auto frame = std::move(_pendingFrames.front());
				_pendingFrames.erase(_pendingFrames.begin());
				return TcpReadResult{TcpReadStatus::Frame, std::move(frame), std::nullopt};
			}
		}
	}

	TcpIoResult TcpConnection::sendFrame(std::string_view payload)
	{
		if (!valid())
		{
			return TcpIoResult{false, makeSocketError(TcpSocketErrorCode::Closed, "connection is closed")};
		}
		auto encoded = encodeFrame(payload, _frameLimits);
		if (!encoded.ok())
		{
			return TcpIoResult{false, makeSocketError(TcpSocketErrorCode::FrameError, encoded.error->message)};
		}

		std::size_t sentBytes = 0;
		while (sentBytes < encoded.bytes.size())
		{
			const int sent = send(toNative(_socket), reinterpret_cast<const char*>(encoded.bytes.data() + sentBytes),
			                      static_cast<int>(encoded.bytes.size() - sentBytes), 0);
			if (sent <= 0)
			{
				if (isTimeoutError())
				{
					return TcpIoResult{false, makeSocketError(TcpSocketErrorCode::TimedOut, "TCP send timed out")};
				}
				close();
				return TcpIoResult{false, makeSocketError(TcpSocketErrorCode::SendFailed, lastSocketErrorMessage())};
			}
			sentBytes += static_cast<std::size_t>(sent);
		}
		return TcpIoResult{true, std::nullopt};
	}

	void TcpConnection::close()
	{
		if (valid())
		{
			closeNative(toNative(_socket));
			_socket = fromNative(InvalidSocket);
		}
	}

	TcpListener::TcpListener(std::intptr_t socketHandle, TcpEndpoint endpoint)
		: _socket(socketHandle),
		  _endpoint(std::move(endpoint))
	{
	}

	TcpListener::TcpListener(TcpListener&& other) noexcept
		: _socket(other._socket),
		  _endpoint(std::move(other._endpoint))
	{
		other._socket = fromNative(InvalidSocket);
	}

	TcpListener& TcpListener::operator=(TcpListener&& other) noexcept
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

	TcpListener::~TcpListener()
	{
		close();
	}

	std::optional<TcpListener> TcpListener::bindAndListen(const TcpEndpoint& endpoint, std::optional<TcpSocketError>& error)
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
				error = makeSocketError(TcpSocketErrorCode::ListenFailed, "failed to listen on TCP endpoint");
				closeNative(socket);
				continue;
			}
			freeaddrinfo(resolved);
			error.reset();
			return TcpListener{fromNative(socket), endpoint};
		}

		freeaddrinfo(resolved);
		error = makeSocketError(TcpSocketErrorCode::BindFailed, "failed to bind TCP endpoint");
		return std::nullopt;
	}

	bool TcpListener::valid() const
	{
		return socketIsValid(_socket);
	}

	std::optional<TcpConnection> TcpListener::accept(std::optional<TcpSocketError>& error)
	{
		if (!valid())
		{
			error = makeSocketError(TcpSocketErrorCode::Closed, "listener is closed");
			return std::nullopt;
		}

		NativeSocket client = ::accept(toNative(_socket), nullptr, nullptr);
		if (client == InvalidSocket)
		{
			if (isTimeoutError())
			{
				error = makeSocketError(TcpSocketErrorCode::TimedOut, "TCP accept timed out");
			}
			else
			{
				error = makeSocketError(TcpSocketErrorCode::AcceptFailed, lastSocketErrorMessage());
			}
			return std::nullopt;
		}
		if (!setTimeouts(client, _endpoint.receiveTimeoutMs, _endpoint.sendTimeoutMs, error))
		{
			closeNative(client);
			return std::nullopt;
		}
		error.reset();
		return TcpConnection{fromNative(client), _endpoint.frameLimits};
	}

	void TcpListener::close()
	{
		if (valid())
		{
			closeNative(toNative(_socket));
			_socket = fromNative(InvalidSocket);
		}
	}
}
