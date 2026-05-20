#include "Session.hpp"
#include "security/TelegramAuth.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <span>
#include <utility>

namespace if_arena::battle_backend
{
	namespace
	{
		BackendResult accepted()
		{
			return BackendResult{true, BackendRejectReason::None};
		}

		BackendResult rejected(BackendRejectReason reason)
		{
			return BackendResult{false, reason};
		}
	}

	bool CreateSessionResult::accepted() const
	{
		return result.accepted;
	}

	BackendSession::BackendSession(SessionId session, ConnectionId connection, IOutboundSession& outbound, BackendLimits limits)
		: _session(session),
		  _connection(connection),
		  _outbound(outbound),
		  _limits(limits)
	{
	}

	SessionId BackendSession::id() const
	{
		return _session;
	}

	ConnectionId BackendSession::connection() const
	{
		return _connection;
	}

	SessionAuthState BackendSession::authState() const
	{
		return _authState;
	}

	std::optional<PlayerId> BackendSession::player() const
	{
		return _player;
	}

	std::size_t BackendSession::pendingOutboundMessages() const
	{
		return _outgoing.size();
	}

	std::size_t BackendSession::pendingOutboundBytes() const
	{
		return _pendingBytes;
	}

	std::uint64_t BackendSession::rejectedBeforeAuthCount() const
	{
		return _rejectedBeforeAuth;
	}

	std::uint64_t BackendSession::queueOverflowCount() const
	{
		return _queueOverflows;
	}

	BackendResult BackendSession::authenticate(PlayerId player)
	{
		if (_authState == SessionAuthState::Closed)
		{
			return rejected(BackendRejectReason::Closed);
		}
		if (player.value == 0)
		{
			return rejected(BackendRejectReason::AuthRequired);
		}

		_player = player;
		_authState = SessionAuthState::Authenticated;
		return accepted();
	}

	BackendResult BackendSession::acceptClientMessage()
	{
		if (_authState == SessionAuthState::Closed)
		{
			return rejected(BackendRejectReason::Closed);
		}
		if (_authState != SessionAuthState::Authenticated)
		{
			++_rejectedBeforeAuth;
			return rejected(BackendRejectReason::AuthRequired);
		}
		return accepted();
	}

	BackendResult BackendSession::enqueueOutbound(std::string payload)
	{
		if (_authState == SessionAuthState::Closed)
		{
			return rejected(BackendRejectReason::Closed);
		}

		const auto nextBytes = _pendingBytes + payload.size();
		if (_outgoing.size() >= _limits.maxPendingOutboundMessages || nextBytes > _limits.maxPendingOutboundBytes)
		{
			++_queueOverflows;
			close(DisconnectReason::QueueOverflow);
			return rejected(BackendRejectReason::QueueFull);
		}

		_pendingBytes = nextBytes;
		_outgoing.push_back(std::move(payload));
		return accepted();
	}

	void BackendSession::flushOutbound()
	{
		if (_authState == SessionAuthState::Closed)
		{
			return;
		}

		for (const auto& payload : _outgoing)
		{
			if (!_outbound.send(payload))
			{
				close(DisconnectReason::QueueOverflow);
				return;
			}
		}
		_outgoing.clear();
		_pendingBytes = 0;
	}

	void BackendSession::close(DisconnectReason reason)
	{
		if (_authState == SessionAuthState::Closed)
		{
			return;
		}

		_authState = SessionAuthState::Closed;
		_outgoing.clear();
		_pendingBytes = 0;
		_outbound.close(reason);
	}

	SessionRegistry::SessionRegistry(BackendLimits limits)
		: _limits(limits)
	{
	}

	CreateSessionResult SessionRegistry::createSession(ConnectionId connection, IOutboundSession& outbound)
	{
		if (_sessions.size() >= _limits.maxSessions)
		{
			return CreateSessionResult{rejected(BackendRejectReason::CapacityReached), std::nullopt};
		}

		const SessionId session{_nextSessionId++};
		_sessions.emplace_back(session, connection, outbound, _limits);
		++_metrics.activeSessions;
		++_metrics.totalCreated;
		return CreateSessionResult{accepted(), session};
	}

	BackendResult SessionRegistry::closeSession(SessionId session, DisconnectReason reason)
	{
		auto* existing = find(session);
		if (existing == nullptr)
		{
			return rejected(BackendRejectReason::NotFound);
		}
		if (existing->authState() != SessionAuthState::Closed)
		{
			existing->close(reason);
			--_metrics.activeSessions;
			++_metrics.totalClosed;
		}
		return accepted();
	}

	BackendSession* SessionRegistry::find(SessionId session)
	{
		const auto found = std::find_if(_sessions.begin(), _sessions.end(), [session](const BackendSession& candidate) {
			return candidate.id() == session;
		});
		return found == _sessions.end() ? nullptr : &*found;
	}

	const BackendSession* SessionRegistry::find(SessionId session) const
	{
		const auto found = std::find_if(_sessions.begin(), _sessions.end(), [session](const BackendSession& candidate) {
			return candidate.id() == session;
		});
		return found == _sessions.end() ? nullptr : &*found;
	}

	SessionMetrics SessionRegistry::metrics() const
	{
		SessionMetrics result = _metrics;
		for (const auto& session : _sessions)
		{
			result.rejectedBeforeAuth += session.rejectedBeforeAuthCount();
			result.outboundQueueOverflows += session.queueOverflowCount();
		}
		return result;
	}
}

namespace if_arena::battle_backend::security
{
	namespace
	{
		using Bytes = std::vector<std::uint8_t>;
		using Digest = std::array<std::uint8_t, 32>;

		constexpr std::array<std::uint32_t, 64> Sha256RoundConstants{
			0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u, 0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
			0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u, 0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
			0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu, 0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
			0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u, 0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
			0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u, 0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
			0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u, 0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
			0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u, 0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
			0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u, 0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u,
		};

		std::uint32_t rotateRight(std::uint32_t value, std::uint32_t bits)
		{
			return (value >> bits) | (value << (32u - bits));
		}

		Bytes toBytes(std::string_view value)
		{
			Bytes bytes;
			bytes.reserve(value.size());
			for (const char ch : value)
			{
				bytes.push_back(static_cast<std::uint8_t>(ch));
			}
			return bytes;
		}

		Digest sha256(std::span<const std::uint8_t> input)
		{
			Bytes message(input.begin(), input.end());
			const std::uint64_t bitLength = static_cast<std::uint64_t>(message.size()) * 8u;
			message.push_back(0x80u);
			while ((message.size() % 64u) != 56u)
			{
				message.push_back(0u);
			}
			for (int shift = 56; shift >= 0; shift -= 8)
			{
				message.push_back(static_cast<std::uint8_t>((bitLength >> static_cast<unsigned int>(shift)) & 0xffu));
			}

			std::array<std::uint32_t, 8> hash{
				0x6a09e667u, 0xbb67ae85u, 0x3c6ef372u, 0xa54ff53au,
				0x510e527fu, 0x9b05688cu, 0x1f83d9abu, 0x5be0cd19u,
			};

			for (std::size_t offset = 0; offset < message.size(); offset += 64u)
			{
				std::array<std::uint32_t, 64> words{};
				for (std::size_t index = 0; index < 16u; ++index)
				{
					const std::size_t base = offset + (index * 4u);
					words[index] = (static_cast<std::uint32_t>(message[base]) << 24u) |
								   (static_cast<std::uint32_t>(message[base + 1u]) << 16u) |
								   (static_cast<std::uint32_t>(message[base + 2u]) << 8u) |
								   static_cast<std::uint32_t>(message[base + 3u]);
				}
				for (std::size_t index = 16u; index < 64u; ++index)
				{
					const auto s0 = rotateRight(words[index - 15u], 7u) ^ rotateRight(words[index - 15u], 18u) ^ (words[index - 15u] >> 3u);
					const auto s1 = rotateRight(words[index - 2u], 17u) ^ rotateRight(words[index - 2u], 19u) ^ (words[index - 2u] >> 10u);
					words[index] = words[index - 16u] + s0 + words[index - 7u] + s1;
				}

				auto a = hash[0];
				auto b = hash[1];
				auto c = hash[2];
				auto d = hash[3];
				auto e = hash[4];
				auto f = hash[5];
				auto g = hash[6];
				auto h = hash[7];

				for (std::size_t index = 0; index < 64u; ++index)
				{
					const auto s1 = rotateRight(e, 6u) ^ rotateRight(e, 11u) ^ rotateRight(e, 25u);
					const auto choice = (e & f) ^ ((~e) & g);
					const auto temp1 = h + s1 + choice + Sha256RoundConstants[index] + words[index];
					const auto s0 = rotateRight(a, 2u) ^ rotateRight(a, 13u) ^ rotateRight(a, 22u);
					const auto majority = (a & b) ^ (a & c) ^ (b & c);
					const auto temp2 = s0 + majority;
					h = g;
					g = f;
					f = e;
					e = d + temp1;
					d = c;
					c = b;
					b = a;
					a = temp1 + temp2;
				}

				hash[0] += a;
				hash[1] += b;
				hash[2] += c;
				hash[3] += d;
				hash[4] += e;
				hash[5] += f;
				hash[6] += g;
				hash[7] += h;
			}

			Digest digest{};
			for (std::size_t index = 0; index < hash.size(); ++index)
			{
				digest[index * 4u] = static_cast<std::uint8_t>((hash[index] >> 24u) & 0xffu);
				digest[(index * 4u) + 1u] = static_cast<std::uint8_t>((hash[index] >> 16u) & 0xffu);
				digest[(index * 4u) + 2u] = static_cast<std::uint8_t>((hash[index] >> 8u) & 0xffu);
				digest[(index * 4u) + 3u] = static_cast<std::uint8_t>(hash[index] & 0xffu);
			}
			return digest;
		}

		Digest hmacSha256(std::string_view key, std::string_view data)
		{
			Bytes keyBytes = toBytes(key);
			if (keyBytes.size() > 64u)
			{
				const auto digest = sha256(keyBytes);
				keyBytes.assign(digest.begin(), digest.end());
			}
			keyBytes.resize(64u, 0u);

			Bytes innerPad(64u);
			Bytes outerPad(64u);
			for (std::size_t index = 0; index < 64u; ++index)
			{
				innerPad[index] = static_cast<std::uint8_t>(keyBytes[index] ^ 0x36u);
				outerPad[index] = static_cast<std::uint8_t>(keyBytes[index] ^ 0x5cu);
			}

			const auto dataBytes = toBytes(data);
			innerPad.insert(innerPad.end(), dataBytes.begin(), dataBytes.end());
			const auto innerDigest = sha256(innerPad);

			outerPad.insert(outerPad.end(), innerDigest.begin(), innerDigest.end());
			return sha256(outerPad);
		}

		std::string hex(Digest digest)
		{
			std::ostringstream output;
			output << std::hex << std::setfill('0');
			for (const auto byte : digest)
			{
				output << std::setw(2) << static_cast<unsigned int>(byte);
			}
			return output.str();
		}

		bool constantTimeEquals(std::string_view lhs, std::string_view rhs)
		{
			if (lhs.size() != rhs.size())
			{
				return false;
			}
			std::uint8_t diff{};
			for (std::size_t index = 0; index < lhs.size(); ++index)
			{
				diff = static_cast<std::uint8_t>(diff | static_cast<std::uint8_t>(lhs[index] ^ rhs[index]));
			}
			return diff == 0u;
		}

		std::optional<std::string> urlDecode(std::string_view value)
		{
			std::string decoded;
			for (std::size_t index = 0; index < value.size(); ++index)
			{
				const char ch = value[index];
				if (ch == '+')
				{
					decoded.push_back(' ');
					continue;
				}
				if (ch != '%')
				{
					decoded.push_back(ch);
					continue;
				}
				if (index + 2u >= value.size())
				{
					return std::nullopt;
				}
				const auto hexValue = value.substr(index + 1u, 2u);
				unsigned int byte{};
				std::istringstream input(std::string{hexValue});
				input >> std::hex >> byte;
				if (input.fail())
				{
					return std::nullopt;
				}
				decoded.push_back(static_cast<char>(byte));
				index += 2u;
			}
			return decoded;
		}

		struct QueryParam
		{
			std::string key;
			std::string value;
		};

		std::optional<std::vector<QueryParam>> parseQuery(std::string_view initData)
		{
			std::vector<QueryParam> params;
			std::size_t start = 0;
			while (start <= initData.size())
			{
				const auto end = initData.find('&', start);
				const auto part = initData.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
				const auto separator = part.find('=');
				if (separator == std::string_view::npos)
				{
					return std::nullopt;
				}
				auto key = urlDecode(part.substr(0, separator));
				auto value = urlDecode(part.substr(separator + 1u));
				if (!key.has_value() || !value.has_value() || key->empty())
				{
					return std::nullopt;
				}
				params.push_back(QueryParam{std::move(*key), std::move(*value)});
				if (end == std::string_view::npos)
				{
					break;
				}
				start = end + 1u;
			}
			return params;
		}

		const std::string* findValue(const std::vector<QueryParam>& params, std::string_view key)
		{
			for (const auto& param : params)
			{
				if (param.key == key)
				{
					return &param.value;
				}
			}
			return nullptr;
		}

		std::optional<std::uint64_t> parseUint64(std::string_view value)
		{
			std::uint64_t parsed{};
			const auto* begin = value.data();
			const auto* end = begin + value.size();
			const auto result = std::from_chars(begin, end, parsed);
			if (result.ec != std::errc{} || result.ptr != end)
			{
				return std::nullopt;
			}
			return parsed;
		}

		std::string dataCheckString(std::vector<QueryParam> params)
		{
			params.erase(std::remove_if(params.begin(), params.end(), [](const QueryParam& param) {
				return param.key == "hash";
			}), params.end());
			std::sort(params.begin(), params.end(), [](const QueryParam& lhs, const QueryParam& rhs) {
				return lhs.key < rhs.key;
			});

			std::string output;
			for (std::size_t index = 0; index < params.size(); ++index)
			{
				if (index != 0)
				{
					output.push_back('\n');
				}
				output += params[index].key;
				output.push_back('=');
				output += params[index].value;
			}
			return output;
		}
	}

	bool TelegramAuthResult::ok() const
	{
		return data.has_value();
	}

	TelegramAuthValidator::TelegramAuthValidator(TelegramAuthConfig config)
		: _config(std::move(config))
	{
	}

	TelegramAuthResult TelegramAuthValidator::validate(std::string_view initData) const
	{
		if (_config.botToken.empty())
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::MissingBotToken};
		}
		if (initData.size() > _config.maxInitDataBytes)
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::OversizedInitData};
		}

		auto params = parseQuery(initData);
		if (!params.has_value())
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::MalformedInitData};
		}

		const auto* receivedHash = findValue(*params, "hash");
		if (receivedHash == nullptr || receivedHash->empty())
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::MissingHash};
		}

		const auto* authDateValue = findValue(*params, "auth_date");
		if (authDateValue == nullptr)
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::MissingAuthDate};
		}
		const auto authDate = parseUint64(*authDateValue);
		if (!authDate.has_value())
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::InvalidAuthDate};
		}
		if (*authDate > _config.nowUnixSeconds || (_config.nowUnixSeconds - *authDate) > _config.maxAgeSeconds)
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::StaleAuthDate};
		}

		const auto secret = hmacSha256("WebAppData", _config.botToken);
		const auto expectedHash = hex(hmacSha256(std::string_view{reinterpret_cast<const char*>(secret.data()), secret.size()}, dataCheckString(*params)));
		if (!constantTimeEquals(*receivedHash, expectedHash))
		{
			return TelegramAuthResult{std::nullopt, TelegramAuthErrorCode::InvalidHash};
		}

		TelegramAuthData data;
		data.authDate = *authDate;
		if (const auto* queryId = findValue(*params, "query_id"); queryId != nullptr)
		{
			data.queryId = *queryId;
		}
		if (const auto* user = findValue(*params, "user"); user != nullptr)
		{
			data.userJson = *user;
		}
		return TelegramAuthResult{std::move(data), TelegramAuthErrorCode::None};
	}

	std::string redactTelegramInitDataForLog(std::string_view initData)
	{
		return initData.empty() ? std::string{} : std::string{"<redacted:telegram_init_data>"};
	}
}
