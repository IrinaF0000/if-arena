#include "Session.hpp"

#include <algorithm>
#include <optional>
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
