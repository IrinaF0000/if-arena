#include "Protocol.hpp"

#include <cctype>
#include <charconv>
#include <limits>
#include <utility>
#include <vector>

namespace if_arena::battle_protocol
{
	namespace
	{
		enum class JsonValueType
		{
			String,
			Integer,
			Object
		};

		struct JsonValue
		{
			JsonValueType type{JsonValueType::String};
			std::string stringValue;
			std::int64_t integerValue{};
			std::string rawJson;
		};

		struct JsonMember
		{
			std::string key;
			JsonValue value;
		};

		ProtocolError makeError(ProtocolErrorCode code, std::string message)
		{
			return ProtocolError{code, std::move(message)};
		}

		ParseResult parseFailure(ProtocolErrorCode code, std::string message)
		{
			return ParseResult{std::nullopt, makeError(code, std::move(message))};
		}

		SerializeResult serializeFailure(ProtocolErrorCode code, std::string message)
		{
			return SerializeResult{std::nullopt, makeError(code, std::move(message))};
		}

		class JsonCursor
		{
		public:
			JsonCursor(std::string_view input, const ProtocolLimits& limits)
				: _input(input),
				  _limits(limits)
			{
			}

			bool parseTopLevelObject(std::vector<JsonMember>& members)
			{
				skipWhitespace();
				if (!consume('{'))
				{
					return fail("expected object");
				}

				skipWhitespace();
				if (consume('}'))
				{
					return true;
				}

				while (!_failed)
				{
					auto key = parseString();
					if (!key.has_value())
					{
						return false;
					}

					skipWhitespace();
					if (!consume(':'))
					{
						return fail("expected field separator");
					}

					auto value = parseValue();
					if (!value.has_value())
					{
						return false;
					}
					members.push_back(JsonMember{std::move(*key), std::move(*value)});

					skipWhitespace();
					if (consume('}'))
					{
						skipWhitespace();
						return _position == _input.size() || fail("unexpected trailing input");
					}
					if (!consume(','))
					{
						return fail("expected field delimiter");
					}
					skipWhitespace();
				}

				return false;
			}

			[[nodiscard]] bool stringTooLarge() const
			{
				return _stringTooLarge;
			}

		private:
			std::string_view _input;
			const ProtocolLimits& _limits;
			std::size_t _position{};
			bool _failed{};
			bool _stringTooLarge{};

			void skipWhitespace()
			{
				while (_position < _input.size() && std::isspace(static_cast<unsigned char>(_input[_position])) != 0)
				{
					++_position;
				}
			}

			bool consume(char expected)
			{
				if (_position < _input.size() && _input[_position] == expected)
				{
					++_position;
					return true;
				}
				return false;
			}

			bool fail(std::string_view)
			{
				_failed = true;
				return false;
			}

			std::optional<std::string> parseString()
			{
				skipWhitespace();
				if (!consume('"'))
				{
					fail("expected string");
					return std::nullopt;
				}

				std::string value;
				while (_position < _input.size())
				{
					const char current = _input[_position++];
					if (current == '"')
					{
						return value;
					}
					if (current == '\\')
					{
						if (_position >= _input.size())
						{
							fail("unterminated escape");
							return std::nullopt;
						}
						const char escaped = _input[_position++];
						if (escaped == '"' || escaped == '\\' || escaped == '/')
						{
							value.push_back(escaped);
						}
						else if (escaped == 'b' || escaped == 'f' || escaped == 'n' || escaped == 'r' || escaped == 't')
						{
							value.push_back(' ');
						}
						else
						{
							fail("unsupported escape");
							return std::nullopt;
						}
					}
					else
					{
						if (static_cast<unsigned char>(current) < 0x20u)
						{
							fail("control character in string");
							return std::nullopt;
						}
						value.push_back(current);
					}

					if (value.size() > _limits.maxStringBytes)
					{
						_stringTooLarge = true;
						fail("string too large");
						return std::nullopt;
					}
				}

				fail("unterminated string");
				return std::nullopt;
			}

			std::optional<JsonValue> parseValue()
			{
				skipWhitespace();
				if (_position >= _input.size())
				{
					fail("expected value");
					return std::nullopt;
				}

				if (_input[_position] == '"')
				{
					auto value = parseString();
					if (!value.has_value())
					{
						return std::nullopt;
					}
					return JsonValue{JsonValueType::String, std::move(*value), 0, {}};
				}
				if (_input[_position] == '{')
				{
					auto raw = parseComposite();
					if (!raw.has_value())
					{
						return std::nullopt;
					}
					return JsonValue{JsonValueType::Object, {}, 0, std::move(*raw)};
				}
				if (_input[_position] == '-' || std::isdigit(static_cast<unsigned char>(_input[_position])) != 0)
				{
					return parseInteger();
				}

				fail("unsupported value");
				return std::nullopt;
			}

			std::optional<JsonValue> parseInteger()
			{
				const auto start = _position;
				if (_position < _input.size() && _input[_position] == '-')
				{
					++_position;
				}
				if (_position >= _input.size() || std::isdigit(static_cast<unsigned char>(_input[_position])) == 0)
				{
					fail("invalid number");
					return std::nullopt;
				}
				while (_position < _input.size() && std::isdigit(static_cast<unsigned char>(_input[_position])) != 0)
				{
					++_position;
				}

				std::int64_t value{};
				const auto number = _input.substr(start, _position - start);
				const auto* begin = number.data();
				const auto* end = begin + number.size();
				const auto result = std::from_chars(begin, end, value);
				if (result.ec != std::errc{} || result.ptr != end)
				{
					fail("invalid number");
					return std::nullopt;
				}
				return JsonValue{JsonValueType::Integer, {}, value, {}};
			}

			std::optional<std::string> parseComposite()
			{
				const auto start = _position;
				std::vector<char> stack;
				while (_position < _input.size())
				{
					const char current = _input[_position++];
					if (current == '"')
					{
						if (!skipStringLiteral())
						{
							return std::nullopt;
						}
						continue;
					}
					if (current == '{' || current == '[')
					{
						stack.push_back(current);
						continue;
					}
					if (current == '}' || current == ']')
					{
						if (stack.empty())
						{
							fail("unbalanced composite");
							return std::nullopt;
						}
						const char expected = current == '}' ? '{' : '[';
						if (stack.back() != expected)
						{
							fail("mismatched composite");
							return std::nullopt;
						}
						stack.pop_back();
						if (stack.empty())
						{
							return std::string{_input.substr(start, _position - start)};
						}
					}
				}

				fail("unterminated composite");
				return std::nullopt;
			}

			bool skipStringLiteral()
			{
				while (_position < _input.size())
				{
					const char current = _input[_position++];
					if (current == '"')
					{
						return true;
					}
					if (current == '\\')
					{
						if (_position >= _input.size())
						{
							return fail("unterminated escape");
						}
						++_position;
					}
					else if (static_cast<unsigned char>(current) < 0x20u)
					{
						return fail("control character in string");
					}
				}
				return fail("unterminated string");
			}
		};

		const JsonValue* findMember(const std::vector<JsonMember>& members, std::string_view key)
		{
			for (const auto& member : members)
			{
				if (member.key == key)
				{
					return &member.value;
				}
			}
			return nullptr;
		}

		ProtocolError okError()
		{
			return ProtocolError{};
		}

		ProtocolError validationError(ProtocolErrorCode code, std::string message)
		{
			return ProtocolError{code, std::move(message)};
		}

		std::optional<std::vector<JsonMember>> parsePayloadObject(std::string_view payloadJson, const ProtocolLimits& limits,
		                                                          ProtocolError& error)
		{
			std::vector<JsonMember> members;
			JsonCursor cursor(payloadJson, limits);
			if (!cursor.parseTopLevelObject(members))
			{
				error = validationError(cursor.stringTooLarge() ? ProtocolErrorCode::StringTooLarge
				                                                : ProtocolErrorCode::MalformedJson,
				                        "invalid payload object");
				return std::nullopt;
			}
			return members;
		}

		bool hasForbiddenAuthorityField(const std::vector<JsonMember>& members)
		{
			for (const auto& member : members)
			{
				if (member.key == "position" || member.key == "hp" || member.key == "cooldown" ||
				    member.key == "damage" || member.key == "score" || member.key == "team" ||
				    member.key == "matchResult" || member.key == "objectiveState" || member.key == "playerId")
				{
					return true;
				}
			}
			return false;
		}

		ProtocolError requireStringField(const std::vector<JsonMember>& members, std::string_view key, std::size_t maxBytes)
		{
			const auto* value = findMember(members, key);
			if (value == nullptr)
			{
				return validationError(ProtocolErrorCode::MissingField, "required payload field is missing");
			}
			if (value->type != JsonValueType::String)
			{
				return validationError(ProtocolErrorCode::InvalidField, "payload field has invalid type");
			}
			if (value->stringValue.empty() || value->stringValue.size() > maxBytes)
			{
				return validationError(ProtocolErrorCode::InvalidField, "payload string field is out of range");
			}
			return okError();
		}

		ProtocolError validateDirectionObject(std::string_view rawJson, const ProtocolLimits& limits)
		{
			ProtocolError error;
			const auto parsed = parsePayloadObject(rawJson, limits, error);
			if (!parsed.has_value())
			{
				return error;
			}
			const auto* x = findMember(*parsed, "x");
			const auto* y = findMember(*parsed, "y");
			if (x == nullptr || y == nullptr)
			{
				return validationError(ProtocolErrorCode::MissingField, "direction is missing coordinates");
			}
			if (x->type != JsonValueType::Integer || y->type != JsonValueType::Integer)
			{
				return validationError(ProtocolErrorCode::InvalidField, "direction coordinates must be integers");
			}
			if (x->integerValue < -1 || x->integerValue > 1 || y->integerValue < -1 || y->integerValue > 1 ||
			    (x->integerValue == 0 && y->integerValue == 0))
			{
				return validationError(ProtocolErrorCode::InvalidField, "direction must be a non-zero unit vector");
			}
			return okError();
		}

		bool isObjectJson(std::string_view value)
		{
			return value.size() >= 2 && value.front() == '{' && value.back() == '}';
		}

		std::string escapeJsonString(std::string_view value)
		{
			std::string escaped;
			for (const char current : value)
			{
				switch (current)
				{
				case '"':
					escaped += "\\\"";
					break;
				case '\\':
					escaped += "\\\\";
					break;
				case '\n':
					escaped += "\\n";
					break;
				case '\r':
					escaped += "\\r";
					break;
				case '\t':
					escaped += "\\t";
					break;
				default:
					if (static_cast<unsigned char>(current) < 0x20u)
					{
						escaped += ' ';
					}
					else
					{
						escaped.push_back(current);
					}
					break;
				}
			}
			return escaped;
		}
	}

	bool ParseResult::ok() const
	{
		return envelope.has_value();
	}

	bool SerializeResult::ok() const
	{
		return json.has_value();
	}

	std::optional<MessageType> messageTypeFromString(std::string_view value)
	{
		if (value == "auth_request")
		{
			return MessageType::AuthRequest;
		}
		if (value == "auth_result")
		{
			return MessageType::AuthResult;
		}
		if (value == "create_match")
		{
			return MessageType::CreateMatch;
		}
		if (value == "join_match")
		{
			return MessageType::JoinMatch;
		}
		if (value == "match_joined")
		{
			return MessageType::MatchJoined;
		}
		if (value == "input_command")
		{
			return MessageType::InputCommand;
		}
		if (value == "input_ack")
		{
			return MessageType::InputAck;
		}
		if (value == "snapshot")
		{
			return MessageType::Snapshot;
		}
		if (value == "event_batch")
		{
			return MessageType::EventBatch;
		}
		if (value == "handshake")
		{
			return MessageType::Handshake;
		}
		if (value == "ping")
		{
			return MessageType::Ping;
		}
		if (value == "pong")
		{
			return MessageType::Pong;
		}
		if (value == "error")
		{
			return MessageType::Error;
		}
		return std::nullopt;
	}

	std::string_view messageTypeName(MessageType type)
	{
		switch (type)
		{
		case MessageType::AuthRequest:
			return "auth_request";
		case MessageType::AuthResult:
			return "auth_result";
		case MessageType::CreateMatch:
			return "create_match";
		case MessageType::JoinMatch:
			return "join_match";
		case MessageType::MatchJoined:
			return "match_joined";
		case MessageType::InputCommand:
			return "input_command";
		case MessageType::InputAck:
			return "input_ack";
		case MessageType::Snapshot:
			return "snapshot";
		case MessageType::EventBatch:
			return "event_batch";
		case MessageType::Handshake:
			return "handshake";
		case MessageType::Ping:
			return "ping";
		case MessageType::Pong:
			return "pong";
		case MessageType::Error:
			return "error";
		}
		return "error";
	}

	ParseResult parseEnvelope(std::string_view input, const ProtocolLimits& limits)
	{
		if (input.size() > limits.maxMessageBytes)
		{
			return parseFailure(ProtocolErrorCode::MessageTooLarge, "message is too large");
		}

		std::vector<JsonMember> members;
		JsonCursor cursor(input, limits);
		if (!cursor.parseTopLevelObject(members))
		{
			const auto code = cursor.stringTooLarge() ? ProtocolErrorCode::StringTooLarge : ProtocolErrorCode::MalformedJson;
			return parseFailure(code, "invalid JSON envelope");
		}

		const auto* version = findMember(members, "version");
		const auto* type = findMember(members, "type");
		const auto* payload = findMember(members, "payload");
		if (version == nullptr || type == nullptr || payload == nullptr)
		{
			return parseFailure(ProtocolErrorCode::MissingField, "required envelope field is missing");
		}
		if (version->type != JsonValueType::Integer || type->type != JsonValueType::String ||
		    payload->type != JsonValueType::Object)
		{
			return parseFailure(ProtocolErrorCode::InvalidField, "required envelope field has invalid type");
		}
		if (version->integerValue != static_cast<std::int64_t>(CurrentProtocolVersion))
		{
			return parseFailure(ProtocolErrorCode::UnsupportedVersion, "unsupported protocol version");
		}
		if (type->stringValue.size() > limits.maxTypeBytes)
		{
			return parseFailure(ProtocolErrorCode::StringTooLarge, "message type is too large");
		}
		const auto messageType = messageTypeFromString(type->stringValue);
		if (!messageType.has_value())
		{
			return parseFailure(ProtocolErrorCode::UnknownType, "unknown message type");
		}
		if (payload->rawJson.size() > limits.maxPayloadBytes)
		{
			return parseFailure(ProtocolErrorCode::PayloadTooLarge, "payload is too large");
		}

		Envelope envelope;
		envelope.version = CurrentProtocolVersion;
		envelope.type = *messageType;
		envelope.typeName = type->stringValue;
		envelope.payloadJson = payload->rawJson;

		if (const auto* requestId = findMember(members, "requestId"); requestId != nullptr)
		{
			if (requestId->type != JsonValueType::String)
			{
				return parseFailure(ProtocolErrorCode::InvalidField, "requestId must be a string");
			}
			if (requestId->stringValue.size() > limits.maxRequestIdBytes)
			{
				return parseFailure(ProtocolErrorCode::StringTooLarge, "requestId is too large");
			}
			envelope.requestId = requestId->stringValue;
		}

		if (const auto* sessionSeq = findMember(members, "sessionSeq"); sessionSeq != nullptr)
		{
			if (sessionSeq->type != JsonValueType::Integer || sessionSeq->integerValue < 0)
			{
				return parseFailure(ProtocolErrorCode::InvalidField, "sessionSeq must be a number");
			}
			envelope.sessionSeq = static_cast<std::uint64_t>(sessionSeq->integerValue);
		}

		return ParseResult{std::move(envelope), {}};
	}

	ProtocolError validateClientEnvelope(const Envelope& envelope, ClientSessionPhase phase, const ProtocolLimits& limits)
	{
		const auto rejectOrder = [] {
			return validationError(ProtocolErrorCode::InvalidMessageOrder, "message is not allowed in current session phase");
		};
		const auto rejectServerMessage = [] {
			return validationError(ProtocolErrorCode::InvalidField, "server-originated message is not accepted from clients");
		};

		switch (envelope.type)
		{
		case MessageType::AuthResult:
		case MessageType::MatchJoined:
		case MessageType::InputAck:
		case MessageType::Snapshot:
		case MessageType::EventBatch:
		case MessageType::Error:
			return rejectServerMessage();
		case MessageType::CreateMatch:
		case MessageType::JoinMatch:
			if (phase != ClientSessionPhase::Authenticated)
			{
				return rejectOrder();
			}
			break;
		case MessageType::AuthRequest:
		case MessageType::Handshake:
			if (phase != ClientSessionPhase::Connected)
			{
				return rejectOrder();
			}
			break;
		case MessageType::InputCommand:
			if (phase != ClientSessionPhase::InMatch)
			{
				return rejectOrder();
			}
			if (!envelope.sessionSeq.has_value())
			{
				return validationError(ProtocolErrorCode::MissingField, "input command requires sessionSeq");
			}
			break;
		case MessageType::Ping:
		case MessageType::Pong:
			break;
		}

		ProtocolError error;
		const auto payload = parsePayloadObject(envelope.payloadJson, limits, error);
		if (!payload.has_value())
		{
			return error;
		}

		if (envelope.type == MessageType::Handshake || envelope.type == MessageType::Ping ||
		    envelope.type == MessageType::Pong)
		{
			return okError();
		}

		if (envelope.type == MessageType::AuthRequest)
		{
			if (auto field = requireStringField(*payload, "mode", 32); field.code != ProtocolErrorCode::None)
			{
				return field;
			}
			const auto* mode = findMember(*payload, "mode");
			if (mode->stringValue == "demo")
			{
				return requireStringField(*payload, "displayName", 64);
			}
			if (mode->stringValue == "telegram")
			{
				return requireStringField(*payload, "initData", 4096);
			}
			return validationError(ProtocolErrorCode::InvalidField, "unknown auth mode");
		}

		if (envelope.type == MessageType::CreateMatch)
		{
			if (auto field = requireStringField(*payload, "mode", 32); field.code != ProtocolErrorCode::None)
			{
				return field;
			}
			if (auto field = requireStringField(*payload, "scenario", 64); field.code != ProtocolErrorCode::None)
			{
				return field;
			}
			const auto* mode = findMember(*payload, "mode");
			const auto* scenario = findMember(*payload, "scenario");
			if (mode->stringValue != "objective_run" || scenario->stringValue != "arena_small_objective_run")
			{
				return validationError(ProtocolErrorCode::InvalidField, "unsupported match configuration");
			}
			return okError();
		}

		if (envelope.type == MessageType::JoinMatch)
		{
			return requireStringField(*payload, "matchCode", 32);
		}

		if (envelope.type == MessageType::InputCommand)
		{
			if (auto field = requireStringField(*payload, "matchId", 64); field.code != ProtocolErrorCode::None)
			{
				return field;
			}
			const auto* command = findMember(*payload, "command");
			if (command == nullptr)
			{
				return validationError(ProtocolErrorCode::MissingField, "input command is missing command object");
			}
			if (command->type != JsonValueType::Object)
			{
				return validationError(ProtocolErrorCode::InvalidField, "command must be an object");
			}

			const auto commandPayload = parsePayloadObject(command->rawJson, limits, error);
			if (!commandPayload.has_value())
			{
				return error;
			}
			if (hasForbiddenAuthorityField(*commandPayload))
			{
				return validationError(ProtocolErrorCode::InvalidField, "client command contains authority field");
			}
			if (auto field = requireStringField(*commandPayload, "kind", 32); field.code != ProtocolErrorCode::None)
			{
				return field;
			}
			const auto* kind = findMember(*commandPayload, "kind");
			const bool needsDirection =
				kind->stringValue == "move" || kind->stringValue == "aim" || kind->stringValue == "attack" ||
				kind->stringValue == "dash";
			if (needsDirection)
			{
				const auto* direction = findMember(*commandPayload, "direction");
				if (direction == nullptr)
				{
					return validationError(ProtocolErrorCode::MissingField, "command direction is missing");
				}
				if (direction->type != JsonValueType::Object)
				{
					return validationError(ProtocolErrorCode::InvalidField, "command direction must be an object");
				}
				return validateDirectionObject(direction->rawJson, limits);
			}
			if (kind->stringValue == "interact" || kind->stringValue == "stop")
			{
				return okError();
			}
			return validationError(ProtocolErrorCode::InvalidField, "unknown command kind");
		}

		return okError();
	}

	SerializeResult serializeEnvelope(const Envelope& envelope, const ProtocolLimits& limits)
	{
		if (envelope.version != CurrentProtocolVersion)
		{
			return serializeFailure(ProtocolErrorCode::UnsupportedVersion, "unsupported protocol version");
		}

		const auto typeName = messageTypeName(envelope.type);
		if (typeName.size() > limits.maxTypeBytes)
		{
			return serializeFailure(ProtocolErrorCode::StringTooLarge, "message type is too large");
		}
		if (!isObjectJson(envelope.payloadJson))
		{
			return serializeFailure(ProtocolErrorCode::InvalidField, "payload must be a JSON object");
		}
		if (envelope.payloadJson.size() > limits.maxPayloadBytes)
		{
			return serializeFailure(ProtocolErrorCode::PayloadTooLarge, "payload is too large");
		}
		if (envelope.requestId.has_value() && envelope.requestId->size() > limits.maxRequestIdBytes)
		{
			return serializeFailure(ProtocolErrorCode::StringTooLarge, "requestId is too large");
		}

		std::string json = "{\"version\":";
		json += std::to_string(CurrentProtocolVersion);
		json += ",\"type\":\"";
		json += typeName;
		json += "\"";
		if (envelope.requestId.has_value())
		{
			json += ",\"requestId\":\"";
			json += escapeJsonString(*envelope.requestId);
			json += "\"";
		}
		if (envelope.sessionSeq.has_value())
		{
			json += ",\"sessionSeq\":";
			json += std::to_string(*envelope.sessionSeq);
		}
		json += ",\"payload\":";
		json += envelope.payloadJson;
		json += "}";

		if (json.size() > limits.maxMessageBytes)
		{
			return serializeFailure(ProtocolErrorCode::MessageTooLarge, "message is too large");
		}
		return SerializeResult{std::move(json), {}};
	}
}
