#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_transport_tcp
{
	struct TcpFrameLimits
	{
		std::uint32_t maxFrameBytes{64u * 1024u};
		std::size_t maxBufferedBytes{128u * 1024u};
	};

	enum class TcpFrameErrorCode
	{
		None,
		InvalidLength,
		FrameTooLarge,
		BufferLimitExceeded,
		DecoderClosed
	};

	struct TcpFrameError
	{
		TcpFrameErrorCode code{TcpFrameErrorCode::None};
		std::string message;
	};

	struct TcpFrameDecodeResult
	{
		std::vector<std::string> frames;
		std::optional<TcpFrameError> error;

		[[nodiscard]] bool ok() const;
	};

	struct TcpFrameEncodeResult
	{
		std::vector<std::uint8_t> bytes;
		std::optional<TcpFrameError> error;

		[[nodiscard]] bool ok() const;
	};

	class TcpFrameDecoder
	{
	public:
		explicit TcpFrameDecoder(TcpFrameLimits limits = {});

		TcpFrameDecodeResult feed(std::span<const std::uint8_t> bytes);
		void reset();

		[[nodiscard]] bool closed() const;
		[[nodiscard]] std::size_t bufferedBytes() const;

	private:
		TcpFrameLimits _limits;
		std::vector<std::uint8_t> _buffer;
		std::optional<TcpFrameError> _error;

		[[nodiscard]] std::uint32_t peekLength() const;
		TcpFrameError fail(TcpFrameErrorCode code, std::string message);
	};

	[[nodiscard]] TcpFrameEncodeResult encodeFrame(std::string_view payload, TcpFrameLimits limits = {});

	struct TcpEndpoint
	{
		std::string host{"127.0.0.1"};
		std::uint16_t port{5555};
		TcpFrameLimits frameLimits;
		std::uint32_t receiveTimeoutMs{5000};
		std::uint32_t sendTimeoutMs{5000};
	};

	enum class TcpSocketErrorCode
	{
		None,
		StartupFailed,
		ResolveFailed,
		SocketFailed,
		BindFailed,
		ListenFailed,
		AcceptFailed,
		ConnectFailed,
		SendFailed,
		ReceiveFailed,
		TimedOut,
		Closed,
		FrameError
	};

	struct TcpSocketError
	{
		TcpSocketErrorCode code{TcpSocketErrorCode::None};
		std::string message;
	};

	enum class TcpReadStatus
	{
		Frame,
		TimedOut,
		Closed,
		Error
	};

	struct TcpReadResult
	{
		TcpReadStatus status{TcpReadStatus::Error};
		std::string frame;
		std::optional<TcpSocketError> error;
	};

	struct TcpIoResult
	{
		bool ok{};
		std::optional<TcpSocketError> error;
	};

	class TcpConnection
	{
	public:
		TcpConnection() = default;
		TcpConnection(const TcpConnection&) = delete;
		TcpConnection& operator=(const TcpConnection&) = delete;
		TcpConnection(TcpConnection&& other) noexcept;
		TcpConnection& operator=(TcpConnection&& other) noexcept;
		~TcpConnection();

		[[nodiscard]] static std::optional<TcpConnection> connectTo(const TcpEndpoint& endpoint,
		                                                            std::optional<TcpSocketError>& error);

		[[nodiscard]] bool valid() const;
		[[nodiscard]] TcpReadResult readFrame();
		[[nodiscard]] TcpIoResult sendFrame(std::string_view payload);
		void close();

	private:
		friend class TcpListener;

		TcpConnection(std::intptr_t socketHandle, TcpFrameLimits limits);

		std::intptr_t _socket{-1};
		TcpFrameLimits _frameLimits;
		TcpFrameDecoder _decoder;
		std::vector<std::string> _pendingFrames;
	};

	class TcpListener
	{
	public:
		TcpListener() = default;
		TcpListener(const TcpListener&) = delete;
		TcpListener& operator=(const TcpListener&) = delete;
		TcpListener(TcpListener&& other) noexcept;
		TcpListener& operator=(TcpListener&& other) noexcept;
		~TcpListener();

		[[nodiscard]] static std::optional<TcpListener> bindAndListen(const TcpEndpoint& endpoint,
		                                                              std::optional<TcpSocketError>& error);

		[[nodiscard]] bool valid() const;
		[[nodiscard]] std::optional<TcpConnection> accept(std::optional<TcpSocketError>& error);
		void close();

	private:
		TcpListener(std::intptr_t socketHandle, TcpEndpoint endpoint);

		std::intptr_t _socket{-1};
		TcpEndpoint _endpoint;
	};
}
