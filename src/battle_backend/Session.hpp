#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_backend
{
	struct SessionId
	{
		std::uint64_t value{};

		friend constexpr bool operator==(SessionId, SessionId) = default;
	};

	struct ConnectionId
	{
		std::uint64_t value{};

		friend constexpr bool operator==(ConnectionId, ConnectionId) = default;
	};

	struct PlayerId
	{
		std::uint64_t value{};

		friend constexpr bool operator==(PlayerId, PlayerId) = default;
	};

	enum class SessionAuthState
	{
		Connected,
		Authenticated,
		Closed
	};

	enum class DisconnectReason
	{
		ClientClosed,
		ProtocolError,
		QueueOverflow,
		ServerShutdown
	};

	enum class BackendRejectReason
	{
		None,
		AuthRequired,
		Closed,
		QueueFull,
		CapacityReached,
		NotFound,
		InvalidMatch,
		MatchNotStarted,
		MatchNotFinished,
		MatchFull,
		InvalidOwnership,
		InvalidSequence,
		RateLimited
	};

	struct BackendResult
	{
		bool accepted{};
		BackendRejectReason reason{BackendRejectReason::None};
	};

	struct CreateSessionResult
	{
		BackendResult result;
		std::optional<SessionId> session;

		[[nodiscard]] bool accepted() const;
	};

	struct BackendLimits
	{
		std::size_t maxSessions{1024};
		std::size_t maxPendingOutboundMessages{64};
		std::size_t maxPendingOutboundBytes{1024u * 1024u};
		std::size_t maxMatches{128};
		std::size_t maxPlayersPerMatch{2};
		std::size_t maxPendingCommandsPerMatch{128};
		std::size_t maxPendingCommandsPerSession{16};
		std::size_t maxCommandsPerSessionPerTick{4};
		std::size_t maxCommandsPerTick{64};
	};

	struct SessionMetrics
	{
		std::uint64_t activeSessions{};
		std::uint64_t totalCreated{};
		std::uint64_t totalClosed{};
		std::uint64_t rejectedBeforeAuth{};
		std::uint64_t outboundQueueOverflows{};
	};

	class IOutboundSession
	{
	public:
		virtual ~IOutboundSession() = default;

		virtual bool send(std::string_view payload) = 0;
		virtual void close(DisconnectReason reason) = 0;
	};

	class BackendSession
	{
	public:
		BackendSession(SessionId session, ConnectionId connection, IOutboundSession& outbound, BackendLimits limits);

		[[nodiscard]] SessionId id() const;
		[[nodiscard]] ConnectionId connection() const;
		[[nodiscard]] SessionAuthState authState() const;
		[[nodiscard]] std::optional<PlayerId> player() const;
		[[nodiscard]] std::size_t pendingOutboundMessages() const;
		[[nodiscard]] std::size_t pendingOutboundBytes() const;
		[[nodiscard]] std::uint64_t rejectedBeforeAuthCount() const;
		[[nodiscard]] std::uint64_t queueOverflowCount() const;

		BackendResult authenticate(PlayerId player);
		BackendResult acceptClientMessage();
		BackendResult enqueueOutbound(std::string payload);
		void flushOutbound();
		void close(DisconnectReason reason);

	private:
		SessionId _session;
		ConnectionId _connection;
		IOutboundSession& _outbound;
		BackendLimits _limits;
		SessionAuthState _authState{SessionAuthState::Connected};
		std::optional<PlayerId> _player;
		std::vector<std::string> _outgoing;
		std::size_t _pendingBytes{};
		std::uint64_t _rejectedBeforeAuth{};
		std::uint64_t _queueOverflows{};
	};

	class SessionRegistry
	{
	public:
		explicit SessionRegistry(BackendLimits limits = {});

		CreateSessionResult createSession(ConnectionId connection, IOutboundSession& outbound);
		BackendResult closeSession(SessionId session, DisconnectReason reason);
		[[nodiscard]] BackendSession* find(SessionId session);
		[[nodiscard]] const BackendSession* find(SessionId session) const;
		[[nodiscard]] SessionMetrics metrics() const;

	private:
		BackendLimits _limits;
		SessionMetrics _metrics;
		std::uint64_t _nextSessionId{1};
		std::vector<BackendSession> _sessions;
	};
}
