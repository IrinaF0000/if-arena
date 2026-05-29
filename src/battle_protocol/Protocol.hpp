#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace if_arena::battle_protocol
{
	constexpr std::uint32_t CurrentProtocolVersion = 1;

	struct ProtocolLimits
	{
		std::size_t maxMessageBytes{64u * 1024u};
		std::size_t maxPayloadBytes{60u * 1024u};
		std::size_t maxStringBytes{4096u};
		std::size_t maxTypeBytes{64u};
		std::size_t maxRequestIdBytes{128u};
	};

	enum class MessageType
	{
		AuthRequest,
		AuthResult,
		CreateMatch,
		JoinMatch,
		StartNextMatch,
		MatchJoined,
		InputCommand,
		InputAck,
		Snapshot,
		EventBatch,
		Handshake,
		Ping,
		Pong,
		Error
	};

	enum class ProtocolErrorCode
	{
		None,
		MessageTooLarge,
		PayloadTooLarge,
		StringTooLarge,
		MalformedJson,
		MissingField,
		InvalidField,
		UnsupportedVersion,
		UnknownType,
		InvalidMessageOrder
	};

	enum class ClientSessionPhase
	{
		Connected,
		Authenticated,
		InMatch
	};

	struct ProtocolError
	{
		ProtocolErrorCode code{ProtocolErrorCode::None};
		std::string message;
	};

	struct Envelope
	{
		std::uint32_t version{CurrentProtocolVersion};
		MessageType type{MessageType::Error};
		std::string typeName;
		std::optional<std::string> requestId;
		std::optional<std::uint64_t> sessionSeq;
		std::string payloadJson;
	};

	struct ParseResult
	{
		std::optional<Envelope> envelope;
		ProtocolError error;

		[[nodiscard]] bool ok() const;
	};

	struct SerializeResult
	{
		std::optional<std::string> json;
		ProtocolError error;

		[[nodiscard]] bool ok() const;
	};

	[[nodiscard]] std::optional<MessageType> messageTypeFromString(std::string_view value);
	[[nodiscard]] std::string_view messageTypeName(MessageType type);
	[[nodiscard]] ParseResult parseEnvelope(std::string_view input, const ProtocolLimits& limits = {});
	[[nodiscard]] ProtocolError validateClientEnvelope(const Envelope& envelope, ClientSessionPhase phase,
	                                                   const ProtocolLimits& limits = {});
	[[nodiscard]] SerializeResult serializeEnvelope(const Envelope& envelope, const ProtocolLimits& limits = {});
}
