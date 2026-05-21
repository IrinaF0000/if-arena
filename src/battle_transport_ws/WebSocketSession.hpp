#pragma once

#include "Protocol.hpp"
#include "Session.hpp"

#include <cstddef>
#include <chrono>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_transport_ws
{
	struct WebSocketLimits
	{
		std::size_t maxMessageBytes{64u * 1024u};
		std::size_t maxPendingMessages{64};
		std::size_t maxPendingBytes{1024u * 1024u};
		std::chrono::milliseconds handshakeTimeout{std::chrono::milliseconds{5000}};
		std::chrono::milliseconds idleTimeout{std::chrono::milliseconds{30000}};
	};

	enum class WebSocketErrorCode
	{
		None,
		MessageTooLarge,
		MalformedMessage,
		InvalidMessageOrder,
		QueueFull,
		HandshakeTimeout,
		IdleTimeout,
		Closed
	};

	struct WebSocketError
	{
		WebSocketErrorCode code{WebSocketErrorCode::None};
		std::string message;
	};

	struct WebSocketMessageResult
	{
		std::optional<battle_protocol::Envelope> envelope;
		std::optional<WebSocketError> error;

		[[nodiscard]] bool ok() const;
	};

	enum class WebSocketLifecycleAction
	{
		None,
		SendPing,
		Close
	};

	struct WebSocketLifecycleResult
	{
		WebSocketLifecycleAction action{WebSocketLifecycleAction::None};
		std::optional<WebSocketError> error;
	};

	struct WebSocketEndpoint
	{
		std::string host{"127.0.0.1"};
		std::uint16_t port{8081};
		std::string path{"/ws"};
		WebSocketLimits limits;
		std::uint32_t receiveTimeoutMs{1000};
		std::uint32_t sendTimeoutMs{5000};
	};

	enum class WebSocketSocketErrorCode
	{
		None,
		StartupFailed,
		ResolveFailed,
		SocketFailed,
		BindFailed,
		ListenFailed,
		AcceptFailed,
		HandshakeFailed,
		SendFailed,
		ReceiveFailed,
		TimedOut,
		Closed,
		FrameError
	};

	struct WebSocketSocketError
	{
		WebSocketSocketErrorCode code{WebSocketSocketErrorCode::None};
		std::string message;
	};

	enum class WebSocketReadStatus
	{
		Text,
		TimedOut,
		Closed,
		Error
	};

	struct WebSocketReadResult
	{
		WebSocketReadStatus status{WebSocketReadStatus::Error};
		std::string text;
		std::optional<WebSocketSocketError> error;
	};

	struct WebSocketIoResult
	{
		bool ok{};
		std::optional<WebSocketSocketError> error;
	};

	class WebSocketFrameDecoder
	{
	public:
		explicit WebSocketFrameDecoder(WebSocketLimits limits = {});

		[[nodiscard]] WebSocketReadResult feed(std::span<const std::uint8_t> bytes);
		[[nodiscard]] bool closed() const;

	private:
		WebSocketLimits _limits;
		std::vector<std::uint8_t> _buffer;
		bool _closed{};

		[[nodiscard]] std::optional<WebSocketReadResult> tryDecode();
		WebSocketReadResult fail(WebSocketSocketErrorCode code, std::string message);
	};

	[[nodiscard]] std::vector<std::uint8_t> encodeWebSocketTextFrame(std::string_view payload);

	class WebSocketSessionAdapter final : public battle_backend::IOutboundSession
	{
	public:
		using Clock = std::chrono::steady_clock;

		explicit WebSocketSessionAdapter(WebSocketLimits limits = {});
		WebSocketSessionAdapter(WebSocketLimits limits, Clock::time_point now);

		WebSocketMessageResult receiveText(std::string_view message, Clock::time_point now = Clock::now());
		WebSocketLifecycleResult checkTimeout(Clock::time_point now);
		void markAuthenticated();
		void markInMatch();
		void markPongReceived(Clock::time_point now = Clock::now());

		bool send(std::string_view payload) override;
		void close(battle_backend::DisconnectReason reason) override;

		[[nodiscard]] bool closed() const;
		[[nodiscard]] battle_backend::DisconnectReason closeReason() const;
		[[nodiscard]] std::size_t sentMessageCount() const;
		[[nodiscard]] std::size_t pendingOutboundBytes() const;
		[[nodiscard]] battle_protocol::ClientSessionPhase phase() const;
		[[nodiscard]] const std::vector<std::string>& sentMessages() const;

	private:
		WebSocketLimits _limits;
		battle_protocol::ClientSessionPhase _phase{battle_protocol::ClientSessionPhase::Connected};
		Clock::time_point _connectedAt;
		Clock::time_point _lastActivityAt;
		bool _closed{};
		bool _pingOutstanding{};
		battle_backend::DisconnectReason _closeReason{battle_backend::DisconnectReason::ClientClosed};
		std::size_t _sentMessageCount{};
		std::size_t _pendingOutboundBytes{};
		std::vector<std::string> _sentMessages;
	};

	class WebSocketConnection
	{
	public:
		WebSocketConnection() = default;
		WebSocketConnection(const WebSocketConnection&) = delete;
		WebSocketConnection& operator=(const WebSocketConnection&) = delete;
		WebSocketConnection(WebSocketConnection&& other) noexcept;
		WebSocketConnection& operator=(WebSocketConnection&& other) noexcept;
		~WebSocketConnection();

		[[nodiscard]] bool valid() const;
		[[nodiscard]] WebSocketReadResult readText();
		[[nodiscard]] WebSocketIoResult sendText(std::string_view payload);
		void close();

	private:
		friend class WebSocketListener;

		WebSocketConnection(std::intptr_t socketHandle, WebSocketLimits limits);

		std::intptr_t _socket{-1};
		WebSocketLimits _limits;
		WebSocketFrameDecoder _decoder;
		std::vector<std::string> _pendingText;
	};

	class WebSocketListener
	{
	public:
		WebSocketListener() = default;
		WebSocketListener(const WebSocketListener&) = delete;
		WebSocketListener& operator=(const WebSocketListener&) = delete;
		WebSocketListener(WebSocketListener&& other) noexcept;
		WebSocketListener& operator=(WebSocketListener&& other) noexcept;
		~WebSocketListener();

		[[nodiscard]] static std::optional<WebSocketListener> bindAndListen(const WebSocketEndpoint& endpoint,
		                                                                    std::optional<WebSocketSocketError>& error);

		[[nodiscard]] bool valid() const;
		[[nodiscard]] std::optional<WebSocketConnection> accept(std::optional<WebSocketSocketError>& error);
		void close();

	private:
		WebSocketListener(std::intptr_t socketHandle, WebSocketEndpoint endpoint);

		std::intptr_t _socket{-1};
		WebSocketEndpoint _endpoint;
	};
}
