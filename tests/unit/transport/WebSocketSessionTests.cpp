#include "WebSocketSession.hpp"

#include <array>
#include <chrono>
#include <cstdint>
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

	std::vector<std::uint8_t> maskedTextFrame(const std::string& payload)
	{
		const std::array<std::uint8_t, 4> mask{1, 2, 3, 4};
		std::vector<std::uint8_t> bytes{0x81u};
		bytes.push_back(static_cast<std::uint8_t>(0x80u | payload.size()));
		bytes.insert(bytes.end(), mask.begin(), mask.end());
		for (std::size_t index = 0; index < payload.size(); ++index)
		{
			bytes.push_back(static_cast<std::uint8_t>(payload[index]) ^ mask[index % mask.size()]);
		}
		return bytes;
	}

	void maskedTextFrameDecodes()
	{
		WebSocketFrameDecoder decoder;
		const auto frame = maskedTextFrame("hello");
		const auto result = decoder.feed(frame);

		require(result.status == WebSocketReadStatus::Text, "masked text frame decoded");
		require(result.text == "hello", "payload unmasked");
	}

	void websocketTextFrameEncodesServerPayload()
	{
		const auto frame = encodeWebSocketTextFrame("hello");

		require(frame.size() == 7, "server text frame size");
		require(frame[0] == 0x81u, "server text fin opcode");
		require(frame[1] == 5u, "server frame is unmasked with small length");
		require(std::string{frame.begin() + 2, frame.end()} == "hello", "server payload preserved");
	}

	void unmaskedClientFrameRejected()
	{
		WebSocketFrameDecoder decoder;
		const std::vector<std::uint8_t> frame{0x81u, 0x02u, 'h', 'i'};
		const auto result = decoder.feed(frame);

		require(result.status == WebSocketReadStatus::Error, "unmasked client frame rejected");
		require(decoder.closed(), "decoder closes on frame error");
	}

	void validMessageParsesThroughSharedProtocol()
	{
		WebSocketSessionAdapter session;
		const auto result =
			session.receiveText("{\"version\":1,\"type\":\"auth_request\",\"payload\":{\"mode\":\"demo\",\"displayName\":\"Ada\"}}");

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
		require(session.closed(), "malformed message closes session");
	}

	void unknownMessageTypeFailsClosed()
	{
		WebSocketSessionAdapter session;

		const auto result = session.receiveText("{\"version\":1,\"type\":\"surprise\",\"payload\":{}}");

		require(!result.ok(), "unknown message rejected");
		require(result.error->code == WebSocketErrorCode::MalformedMessage, "unknown type maps to malformed boundary error");
		require(session.closed(), "unknown type closes session");
	}

	void gameplayBeforeAuthFailsClosed()
	{
		WebSocketSessionAdapter session;

		const auto result = session.receiveText(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":1,\"payload\":{\"matchId\":\"1\",\"command\":{\"kind\":\"stop\"}}}");

		require(!result.ok(), "gameplay before auth rejected");
		require(result.error->code == WebSocketErrorCode::InvalidMessageOrder, "message order error code");
		require(session.closed(), "invalid phase closes session");
	}

	void phaseAdvancesAfterBackendAuthAndJoin()
	{
		WebSocketSessionAdapter session;
		const auto auth =
			session.receiveText("{\"version\":1,\"type\":\"auth_request\",\"payload\":{\"mode\":\"demo\",\"displayName\":\"Ada\"}}");
		require(auth.ok(), "auth request accepted in connected phase");

		session.markAuthenticated();
		const auto create = session.receiveText(
			"{\"version\":1,\"type\":\"create_match\",\"payload\":{\"mode\":\"objective_run\",\"scenario\":\"arena_small_objective_run\"}}");
		require(create.ok(), "create match accepted after auth");

		session.markInMatch();
		const auto input = session.receiveText(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":1,\"payload\":{\"matchId\":\"1\",\"command\":{\"kind\":\"stop\"}}}");
		require(input.ok(), "input command accepted in match phase");
	}

	void outboundSendIsBounded()
	{
		WebSocketLimits limits;
		limits.maxMessageBytes = 4;
		WebSocketSessionAdapter session(limits);

		require(session.send("pong"), "small outbound message accepted");
		require(!session.send("too-large"), "oversized outbound message rejected");
		require(session.sentMessageCount() == 1, "only accepted outbound message counted");
		require(session.pendingOutboundBytes() == 4, "pending outbound bytes tracked");
	}

	void outboundQueueLimitFailsClosed()
	{
		WebSocketLimits limits;
		limits.maxPendingMessages = 1;
		WebSocketSessionAdapter session(limits);

		require(session.send("one"), "first message accepted");
		require(!session.send("two"), "second message rejected by queue bound");
		require(session.closed(), "queue overflow closes session");
		require(session.closeReason() == if_arena::battle_backend::DisconnectReason::QueueOverflow, "queue overflow reason");
	}

	void handshakeAndIdleTimeoutsAreExplicit()
	{
		using Clock = WebSocketSessionAdapter::Clock;
		const auto start = Clock::time_point{std::chrono::milliseconds{0}};
		WebSocketLimits limits;
		limits.handshakeTimeout = std::chrono::milliseconds{100};
		limits.idleTimeout = std::chrono::milliseconds{50};

		WebSocketSessionAdapter handshakeSession(limits, start);
		const auto handshake = handshakeSession.checkTimeout(start + std::chrono::milliseconds{100});
		require(handshake.action == WebSocketLifecycleAction::Close, "handshake timeout closes");
		require(handshake.error->code == WebSocketErrorCode::HandshakeTimeout, "handshake timeout code");

		WebSocketSessionAdapter idleSession(limits, start);
		idleSession.markAuthenticated();
		const auto ping = idleSession.checkTimeout(start + std::chrono::milliseconds{50});
		require(ping.action == WebSocketLifecycleAction::SendPing, "idle timeout first sends ping");
		const auto close = idleSession.checkTimeout(start + std::chrono::milliseconds{100});
		require(close.action == WebSocketLifecycleAction::Close, "unanswered ping closes");
		require(close.error->code == WebSocketErrorCode::IdleTimeout, "idle timeout code");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"maskedTextFrameDecodes", maskedTextFrameDecodes},
		{"websocketTextFrameEncodesServerPayload", websocketTextFrameEncodesServerPayload},
		{"unmaskedClientFrameRejected", unmaskedClientFrameRejected},
		{"validMessageParsesThroughSharedProtocol", validMessageParsesThroughSharedProtocol},
		{"oversizedMessageRejectedAndClosed", oversizedMessageRejectedAndClosed},
		{"malformedMessageDoesNotCrash", malformedMessageDoesNotCrash},
		{"unknownMessageTypeFailsClosed", unknownMessageTypeFailsClosed},
		{"gameplayBeforeAuthFailsClosed", gameplayBeforeAuthFailsClosed},
		{"phaseAdvancesAfterBackendAuthAndJoin", phaseAdvancesAfterBackendAuthAndJoin},
		{"outboundSendIsBounded", outboundSendIsBounded},
		{"outboundQueueLimitFailsClosed", outboundQueueLimitFailsClosed},
		{"handshakeAndIdleTimeoutsAreExplicit", handshakeAndIdleTimeoutsAreExplicit},
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
