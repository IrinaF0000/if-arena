#pragma once

#include "Protocol.hpp"
#include "Session.hpp"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace if_arena::battle_transport_ws
{
	struct WebSocketLimits
	{
		std::size_t maxMessageBytes{64u * 1024u};
	};

	enum class WebSocketErrorCode
	{
		None,
		MessageTooLarge,
		MalformedMessage,
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

	class WebSocketSessionAdapter final : public battle_backend::IOutboundSession
	{
	public:
		explicit WebSocketSessionAdapter(WebSocketLimits limits = {});

		WebSocketMessageResult receiveText(std::string_view message);

		bool send(std::string_view payload) override;
		void close(battle_backend::DisconnectReason reason) override;

		[[nodiscard]] bool closed() const;
		[[nodiscard]] battle_backend::DisconnectReason closeReason() const;
		[[nodiscard]] std::size_t sentMessageCount() const;

	private:
		WebSocketLimits _limits;
		bool _closed{};
		battle_backend::DisconnectReason _closeReason{battle_backend::DisconnectReason::ClientClosed};
		std::size_t _sentMessageCount{};
	};
}
