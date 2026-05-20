#include "Session.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	using namespace if_arena::battle_backend;

	class FakeOutboundSession final : public IOutboundSession
	{
	public:
		bool send(std::string_view payload) override
		{
			sent.emplace_back(payload);
			return sendSucceeds;
		}

		void close(DisconnectReason reason) override
		{
			closed = true;
			closeReason = reason;
		}

		bool sendSucceeds{true};
		bool closed{};
		DisconnectReason closeReason{DisconnectReason::ClientClosed};
		std::vector<std::string> sent;
	};

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void sessionCanBeCreatedAndClosed()
	{
		FakeOutboundSession outbound;
		SessionRegistry registry;

		const auto created = registry.createSession(ConnectionId{7}, outbound);
		require(created.accepted(), "session creation accepted");
		require(created.session.has_value(), "created session id returned");

		auto* session = registry.find(*created.session);
		require(session != nullptr, "created session is findable");
		require(session->connection() == ConnectionId{7}, "connection id preserved");
		require(session->authState() == SessionAuthState::Connected, "new session starts connected");

		const auto metricsAfterCreate = registry.metrics();
		require(metricsAfterCreate.activeSessions == 1, "active session metric increments");
		require(metricsAfterCreate.totalCreated == 1, "total created metric increments");

		const auto closed = registry.closeSession(*created.session, DisconnectReason::ServerShutdown);
		require(closed.accepted, "session close accepted");
		require(outbound.closed, "transport close callback invoked");
		require(outbound.closeReason == DisconnectReason::ServerShutdown, "close reason preserved");

		const auto metricsAfterClose = registry.metrics();
		require(metricsAfterClose.activeSessions == 0, "active session metric decrements");
		require(metricsAfterClose.totalClosed == 1, "total closed metric increments");
	}

	void commandBeforeAuthRejected()
	{
		FakeOutboundSession outbound;
		SessionRegistry registry;
		const auto created = registry.createSession(ConnectionId{1}, outbound);
		require(created.session.has_value(), "created session id returned");
		auto* session = registry.find(*created.session);
		require(session != nullptr, "session exists");

		const auto beforeAuth = session->acceptClientMessage();
		require(!beforeAuth.accepted, "message before auth rejected");
		require(beforeAuth.reason == BackendRejectReason::AuthRequired, "auth required reason returned");

		const auto auth = session->authenticate(PlayerId{42});
		require(auth.accepted, "auth accepted");
		require(session->authState() == SessionAuthState::Authenticated, "session becomes authenticated");
		require(session->player() == PlayerId{42}, "player id is backend-owned state");

		const auto afterAuth = session->acceptClientMessage();
		require(afterAuth.accepted, "message after auth accepted");
		require(registry.metrics().rejectedBeforeAuth == 1, "rejected before auth metric increments");
	}

	void outgoingQueueIsBounded()
	{
		BackendLimits limits;
		limits.maxPendingOutboundMessages = 1;
		limits.maxPendingOutboundBytes = 64;

		FakeOutboundSession outbound;
		SessionRegistry registry(limits);
		const auto created = registry.createSession(ConnectionId{1}, outbound);
		require(created.session.has_value(), "created session id returned");
		auto* session = registry.find(*created.session);
		require(session != nullptr, "session exists");

		const auto first = session->enqueueOutbound("snapshot-1");
		require(first.accepted, "first outbound message queued");
		require(session->pendingOutboundMessages() == 1, "pending message count tracked");

		const auto second = session->enqueueOutbound("snapshot-2");
		require(!second.accepted, "second outbound message rejected by queue bound");
		require(second.reason == BackendRejectReason::QueueFull, "queue full reason returned");
		require(outbound.closed, "slow/overflowing transport is closed");
		require(outbound.closeReason == DisconnectReason::QueueOverflow, "queue overflow close reason");
		require(registry.metrics().outboundQueueOverflows == 1, "queue overflow metric increments");
	}

	void flushSendsQueuedMessages()
	{
		FakeOutboundSession outbound;
		SessionRegistry registry;
		const auto created = registry.createSession(ConnectionId{1}, outbound);
		require(created.session.has_value(), "created session id returned");
		auto* session = registry.find(*created.session);
		require(session != nullptr, "session exists");

		session->enqueueOutbound("a");
		session->enqueueOutbound("b");
		session->flushOutbound();

		require(outbound.sent.size() == 2, "queued messages sent");
		require(outbound.sent.front() == "a", "first message preserved");
		require(session->pendingOutboundMessages() == 0, "queue cleared after flush");
		require(session->pendingOutboundBytes() == 0, "pending bytes cleared after flush");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"sessionCanBeCreatedAndClosed", sessionCanBeCreatedAndClosed},
		{"commandBeforeAuthRejected", commandBeforeAuthRejected},
		{"outgoingQueueIsBounded", outgoingQueueIsBounded},
		{"flushSendsQueuedMessages", flushSendsQueuedMessages},
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
