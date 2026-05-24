#include "Protocol.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_protocol;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void validEnvelopeParses()
	{
		const auto result = parseEnvelope(
			"{\"version\":1,\"type\":\"input_command\",\"requestId\":\"r1\",\"sessionSeq\":7,\"payload\":{\"matchId\":\"m1\"}}");

		require(result.ok(), "valid envelope parses");
		require(result.envelope->type == MessageType::InputCommand, "message type parsed");
		require(result.envelope->requestId == std::string{"r1"}, "requestId parsed");
		require(result.envelope->sessionSeq == 7u, "sessionSeq parsed");
		require(result.envelope->payloadJson == "{\"matchId\":\"m1\"}", "payload object preserved");
	}

	void serializerRoundTrips()
	{
		Envelope envelope;
		envelope.type = MessageType::AuthRequest;
		envelope.requestId = "r2";
		envelope.payloadJson = "{\"mode\":\"demo\"}";

		const auto serialized = serializeEnvelope(envelope);
		require(serialized.ok(), "envelope serializes");

		const auto parsed = parseEnvelope(*serialized.json);
		require(parsed.ok(), "serialized envelope parses");
		require(parsed.envelope->type == MessageType::AuthRequest, "roundtrip type preserved");
		require(parsed.envelope->payloadJson == envelope.payloadJson, "roundtrip payload preserved");
	}

	void malformedJsonRejected()
	{
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"input_command\",\"payload\":{}");

		require(!result.ok(), "malformed JSON rejected");
		require(result.error.code == ProtocolErrorCode::MalformedJson, "malformed JSON error code");
	}

	void unknownTypeRejected()
	{
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"surprise\",\"payload\":{}}");

		require(!result.ok(), "unknown type rejected");
		require(result.error.code == ProtocolErrorCode::UnknownType, "unknown type error code");
	}

	void oversizedStringRejected()
	{
		ProtocolLimits limits;
		limits.maxStringBytes = 8;
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"input_command\",\"requestId\":\"this-is-too-long\",\"payload\":{}}", limits);

		require(!result.ok(), "oversized string rejected");
		require(result.error.code == ProtocolErrorCode::StringTooLarge, "oversized string error code");
	}

	void missingFieldRejected()
	{
		const auto result = parseEnvelope("{\"version\":1,\"payload\":{}}");

		require(!result.ok(), "missing field rejected");
		require(result.error.code == ProtocolErrorCode::MissingField, "missing field error code");
	}

	void unsupportedVersionRejected()
	{
		const auto result = parseEnvelope("{\"version\":2,\"type\":\"input_command\",\"payload\":{}}");

		require(!result.ok(), "unsupported version rejected");
		require(result.error.code == ProtocolErrorCode::UnsupportedVersion, "unsupported version error code");
	}

	void invalidFieldTypeRejected()
	{
		const auto result = parseEnvelope("{\"version\":\"1\",\"type\":\"input_command\",\"payload\":{}}");

		require(!result.ok(), "invalid field type rejected");
		require(result.error.code == ProtocolErrorCode::InvalidField, "invalid field type error code");
	}

	void wrongPayloadTypeRejected()
	{
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"input_command\",\"payload\":\"not-an-object\"}");

		require(!result.ok(), "wrong payload type rejected");
		require(result.error.code == ProtocolErrorCode::InvalidField, "wrong payload type error code");
	}

	void negativeSessionSeqRejected()
	{
		const auto result = parseEnvelope(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":-1,\"payload\":{\"matchId\":\"m1\"}}");

		require(!result.ok(), "negative sessionSeq rejected");
		require(result.error.code == ProtocolErrorCode::InvalidField, "negative sessionSeq error code");
	}

	void oversizedMessageRejectedBeforeParsing()
	{
		ProtocolLimits limits;
		limits.maxMessageBytes = 16;
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"input_command\",\"payload\":{}}", limits);

		require(!result.ok(), "oversized message rejected");
		require(result.error.code == ProtocolErrorCode::MessageTooLarge, "oversized message error code");
	}

	Envelope mustParse(std::string_view json)
	{
		const auto result = parseEnvelope(json);
		require(result.ok(), "test envelope parses");
		return *result.envelope;
	}

	void playableMessageTypesRoundTrip()
	{
		require(messageTypeFromString("handshake") == MessageType::Handshake, "handshake type recognized");
		require(messageTypeFromString("ping") == MessageType::Ping, "ping type recognized");
		require(messageTypeFromString("pong") == MessageType::Pong, "pong type recognized");
		require(messageTypeName(MessageType::Handshake) == "handshake", "handshake name serialized");
		require(messageTypeName(MessageType::Ping) == "ping", "ping name serialized");
		require(messageTypeName(MessageType::Pong) == "pong", "pong name serialized");
	}

	void demoAuthPayloadValidates()
	{
		const auto envelope =
			mustParse("{\"version\":1,\"type\":\"auth_request\",\"payload\":{\"mode\":\"demo\",\"displayName\":\"Ana\"}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::Connected);

		require(error.code == ProtocolErrorCode::None, "demo auth payload validates");
	}

	void inputCommandPayloadValidatesIntentOnly()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":9,\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"attack\",\"direction\":{\"x\":1,\"y\":0}}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::None, "intent-only attack command validates");
	}

	void authorityFieldsRejected()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":9,\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"move\",\"direction\":{\"x\":1,\"y\":0},\"hp\":100}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::InvalidField, "client authority field rejected");
	}

	void invalidCommandEnumRejected()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":9,\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"teleport\",\"direction\":{\"x\":1,\"y\":0}}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::InvalidField, "invalid command kind rejected");
	}

	void invalidDirectionRejected()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":9,\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"dash\",\"direction\":{\"x\":2,\"y\":0}}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::InvalidField, "out-of-range direction rejected");
	}

	void inputCommandRequiresSessionSeq()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"interact\"}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::MissingField, "input command without sessionSeq rejected");
	}

	void commandBeforeJoinRejected()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"input_command\",\"sessionSeq\":1,\"payload\":{\"matchId\":\"m1\",\"command\":{\"kind\":\"interact\"}}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::Authenticated);

		require(error.code == ProtocolErrorCode::InvalidMessageOrder, "command before joining match rejected");
	}

	void createMatchAfterJoinRejected()
	{
		const auto envelope = mustParse(
			"{\"version\":1,\"type\":\"create_match\",\"payload\":{\"mode\":\"objective_run\",\"scenario\":\"arena_small_objective_run\"}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::InvalidMessageOrder, "create match after joining is rejected");
	}

	void repeatedAuthRejected()
	{
		const auto envelope =
			mustParse("{\"version\":1,\"type\":\"auth_request\",\"payload\":{\"mode\":\"demo\",\"displayName\":\"Ana\"}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::Authenticated);

		require(error.code == ProtocolErrorCode::InvalidMessageOrder, "repeated auth after authentication rejected");
	}

	void serverMessagesRejectedFromClient()
	{
		const auto envelope = mustParse("{\"version\":1,\"type\":\"snapshot\",\"payload\":{}}");

		const auto error = validateClientEnvelope(envelope, ClientSessionPhase::InMatch);

		require(error.code == ProtocolErrorCode::InvalidField, "server-originated message rejected from client");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"validEnvelopeParses", validEnvelopeParses},
		{"serializerRoundTrips", serializerRoundTrips},
		{"malformedJsonRejected", malformedJsonRejected},
		{"unknownTypeRejected", unknownTypeRejected},
		{"oversizedStringRejected", oversizedStringRejected},
		{"missingFieldRejected", missingFieldRejected},
		{"unsupportedVersionRejected", unsupportedVersionRejected},
		{"invalidFieldTypeRejected", invalidFieldTypeRejected},
		{"wrongPayloadTypeRejected", wrongPayloadTypeRejected},
		{"negativeSessionSeqRejected", negativeSessionSeqRejected},
		{"oversizedMessageRejectedBeforeParsing", oversizedMessageRejectedBeforeParsing},
		{"playableMessageTypesRoundTrip", playableMessageTypesRoundTrip},
		{"demoAuthPayloadValidates", demoAuthPayloadValidates},
		{"inputCommandPayloadValidatesIntentOnly", inputCommandPayloadValidatesIntentOnly},
		{"authorityFieldsRejected", authorityFieldsRejected},
		{"invalidCommandEnumRejected", invalidCommandEnumRejected},
		{"invalidDirectionRejected", invalidDirectionRejected},
		{"inputCommandRequiresSessionSeq", inputCommandRequiresSessionSeq},
		{"commandBeforeJoinRejected", commandBeforeJoinRejected},
		{"createMatchAfterJoinRejected", createMatchAfterJoinRejected},
		{"repeatedAuthRejected", repeatedAuthRejected},
		{"serverMessagesRejectedFromClient", serverMessagesRejectedFromClient},
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
