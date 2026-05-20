#include "WebSocketSession.hpp"

#include <utility>

namespace if_arena::battle_transport_ws
{
	namespace
	{
		WebSocketError makeError(WebSocketErrorCode code, std::string message)
		{
			return WebSocketError{code, std::move(message)};
		}
	}

	bool WebSocketMessageResult::ok() const
	{
		return envelope.has_value();
	}

	WebSocketSessionAdapter::WebSocketSessionAdapter(WebSocketLimits limits)
		: _limits(limits)
	{
	}

	WebSocketMessageResult WebSocketSessionAdapter::receiveText(std::string_view message)
	{
		if (_closed)
		{
			return WebSocketMessageResult{std::nullopt, makeError(WebSocketErrorCode::Closed, "session is closed")};
		}
		if (message.size() > _limits.maxMessageBytes)
		{
			close(battle_backend::DisconnectReason::ProtocolError);
			return WebSocketMessageResult{std::nullopt, makeError(WebSocketErrorCode::MessageTooLarge, "message is too large")};
		}

		battle_protocol::ProtocolLimits protocolLimits;
		protocolLimits.maxMessageBytes = _limits.maxMessageBytes;
		const auto parsed = battle_protocol::parseEnvelope(message, protocolLimits);
		if (!parsed.ok())
		{
			return WebSocketMessageResult{std::nullopt, makeError(WebSocketErrorCode::MalformedMessage, "invalid protocol message")};
		}

		return WebSocketMessageResult{parsed.envelope, std::nullopt};
	}

	bool WebSocketSessionAdapter::send(std::string_view payload)
	{
		if (_closed || payload.size() > _limits.maxMessageBytes)
		{
			return false;
		}

		++_sentMessageCount;
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
}
