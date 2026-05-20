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

	void invalidFieldTypeRejected()
	{
		const auto result = parseEnvelope("{\"version\":\"1\",\"type\":\"input_command\",\"payload\":{}}");

		require(!result.ok(), "invalid field type rejected");
		require(result.error.code == ProtocolErrorCode::InvalidField, "invalid field type error code");
	}

	void oversizedMessageRejectedBeforeParsing()
	{
		ProtocolLimits limits;
		limits.maxMessageBytes = 16;
		const auto result = parseEnvelope("{\"version\":1,\"type\":\"input_command\",\"payload\":{}}", limits);

		require(!result.ok(), "oversized message rejected");
		require(result.error.code == ProtocolErrorCode::MessageTooLarge, "oversized message error code");
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
		{"invalidFieldTypeRejected", invalidFieldTypeRejected},
		{"oversizedMessageRejectedBeforeParsing", oversizedMessageRejectedBeforeParsing},
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
