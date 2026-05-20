#include "WebSocketSession.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_transport_ws;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void validMessageParsesThroughSharedProtocol()
	{
		WebSocketSessionAdapter session;
		const auto result = session.receiveText("{\"version\":1,\"type\":\"auth_request\",\"payload\":{\"mode\":\"demo\"}}");

		require(result.ok(), "valid WebSocket protocol message parses");
		require(result.envelope->type == if_arena::battle_protocol::MessageType::AuthRequest, "message type preserved");
		require(!session.closed(), "valid message does not close session");
	}

	void oversizedMessageRejectedAndClosed()
	{
		WebSocketLimits limits;
		limits.maxMessageBytes = 8;
		WebSocketSessionAdapter session(limits);

		const auto result = session.receiveText("{\"too\":\"large\"}");

		require(!result.ok(), "oversized message rejected");
		require(result.error->code == WebSocketErrorCode::MessageTooLarge, "oversized error code");
		require(session.closed(), "oversized message closes session");
		require(session.closeReason() == if_arena::battle_backend::DisconnectReason::ProtocolError, "protocol close reason");
	}

	void malformedMessageDoesNotCrash()
	{
		WebSocketSessionAdapter session;

		const auto result = session.receiveText("{\"version\":1,\"type\":\"auth_request\",\"payload\":{}");

		require(!result.ok(), "malformed message rejected");
		require(result.error->code == WebSocketErrorCode::MalformedMessage, "malformed error code");
		require(!session.closed(), "malformed message result is explicit and non-crashing");
	}

	void outboundSendIsBounded()
	{
		WebSocketLimits limits;
		limits.maxMessageBytes = 4;
		WebSocketSessionAdapter session(limits);

		require(session.send("pong"), "small outbound message accepted");
		require(!session.send("too-large"), "oversized outbound message rejected");
		require(session.sentMessageCount() == 1, "only accepted outbound message counted");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"validMessageParsesThroughSharedProtocol", validMessageParsesThroughSharedProtocol},
		{"oversizedMessageRejectedAndClosed", oversizedMessageRejectedAndClosed},
		{"malformedMessageDoesNotCrash", malformedMessageDoesNotCrash},
		{"outboundSendIsBounded", outboundSendIsBounded},
	};

	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
			std::cout << "[PASS] " << name << '\n';
		}
		catch (const std::exception& ex)
		{
			++failed;
			std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
		}
	}

	if (failed != 0)
	{
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}

	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
