#include "Session.hpp"
#include "MatchLoop.hpp"
#include "ScenarioConfig.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
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

	std::string readRepoFile(const std::filesystem::path& relative)
	{
		for (const auto& root : {std::filesystem::path{"."}, std::filesystem::path{".."}})
		{
			const auto path = root / relative;
			std::ifstream input(path);
			if (input)
			{
				std::ostringstream buffer;
				buffer << input.rdbuf();
				return buffer.str();
			}
		}
		throw std::runtime_error("unable to read " + relative.string());
	}

	void scenarioConfigLoadsPlayableArena()
	{
		const auto loaded = parseScenarioConfig(readRepoFile("config/scenarios/arena_small_objective_run.json"));
		require(loaded.ok(), "playable scenario config parses");
		const auto& scenario = *loaded.scenario;
		require(scenario.id == "arena_small_objective_run", "scenario id preserved");
		require(scenario.arena.obstacles.size() == 16, "scenario obstacles are loaded from config");
		require(scenario.arena.hazards.size() == 5, "scenario hazards are loaded from config");
		require(scenario.arena.hazards.front().id == "tower_left", "hazard id is loaded from config");
		require(scenario.arena.hazards.front().effect == if_arena::battle_core::HazardEffect::DamageAndDropObjective,
		        "hazard effect is loaded from config");
		require(scenario.arena.hazards.front().trigger == if_arena::battle_core::HazardTrigger::Range,
		        "hazard trigger is loaded from config");
		require(scenario.arena.hazards.front().icon == "hazard_tower", "hazard icon is loaded from config");
		require(scenario.maxTicks == 3600, "scenario time limit converts to ticks");
	}

	void scenarioConfigRejectsInvalidArena()
	{
		const auto loaded = parseScenarioConfig(readRepoFile("config/test_scenarios/invalid_asymmetric_obstacle.json"));
		require(!loaded.ok(), "invalid scenario config is rejected");
		require(std::any_of(loaded.errors.begin(), loaded.errors.end(), [](const std::string& error) {
			        return error.find("rotational symmetry") != std::string::npos;
		        }),
		        "invalid scenario reports symmetry failure");
	}

	void scenarioConfigRejectsInvalidHazardMetadata()
	{
		auto text = readRepoFile("config/scenarios/arena_small_objective_run.json");
		const auto position = text.find("\"effect\": \"damage_drop_objective\"");
		require(position != std::string::npos, "fixture contains hazard effect metadata");
		text.replace(position, std::string{"\"effect\": \"damage_drop_objective\""}.size(), "\"effect\": \"teleport\"");

		const auto loaded = parseScenarioConfig(text);

		require(!loaded.ok(), "invalid hazard metadata is rejected");
		require(std::any_of(loaded.errors.begin(), loaded.errors.end(), [](const std::string& error) {
			        return error.find("unsupported hazard effect") != std::string::npos;
		        }),
		        "invalid hazard metadata reports effect failure");
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

	struct MatchHarness
	{
		BackendLimits limits{};
		FakeOutboundSession blueOutbound;
		FakeOutboundSession redOutbound;
		SessionRegistry registry{limits};
		MatchManager manager{registry, limits};
		SessionId blue{};
		SessionId red{};
		MatchId match{};

		MatchHarness()
		{
			const auto blueSession = registry.createSession(ConnectionId{1}, blueOutbound);
			const auto redSession = registry.createSession(ConnectionId{2}, redOutbound);
			require(blueSession.session.has_value(), "blue session created");
			require(redSession.session.has_value(), "red session created");
			blue = *blueSession.session;
			red = *redSession.session;
			require(registry.find(blue)->authenticate(PlayerId{11}).accepted, "blue authenticated");
			require(registry.find(red)->authenticate(PlayerId{22}).accepted, "red authenticated");

			const auto created = manager.createMatch(blue);
			require(created.accepted(), "match created");
			const auto joined = manager.joinMatch(red, created.joinCode);
			require(joined.accepted(), "red joined match");
			require(joined.match.has_value(), "joined match id returned");
			match = *joined.match;
		}
	};

	void matchStartsThroughJoinCode()
	{
		MatchHarness harness;

		const auto view = harness.manager.view(harness.match);
		require(view.has_value(), "match view exists");
		require(view->started, "match starts after two players join");
		require(view->playerCount == 2, "two players in match");
		require(view->snapshot.has_value(), "started match has snapshot");
		require(view->snapshot->players.size() == 2, "snapshot contains both players");
		require(!view->snapshot->obstacles.empty(), "snapshot contains authoritative obstacle cells");
		require(harness.manager.metrics().activeMatches == 1, "active match metric increments");
		require(harness.manager.metrics().matchesCreated == 1, "created match metric increments");
	}

	void commandsAreOwnedSequencedAndBroadcast()
	{
		MatchHarness harness;

		const BackendCommand moveNorth{BackendCommandKind::Move, {0, -1}};
		const auto queued = harness.manager.submitCommand(harness.blue, harness.match, 1, moveNorth, PlayerId{11});
		require(queued.accepted, "owned command accepted into queue");

		const auto duplicate = harness.manager.submitCommand(harness.blue, harness.match, 1, moveNorth, PlayerId{11});
		require(!duplicate.accepted, "duplicate sequence rejected");
		require(duplicate.reason == BackendRejectReason::InvalidSequence, "duplicate sequence reason");

		const auto wrongPlayer = harness.manager.submitCommand(harness.blue, harness.match, 2, moveNorth, PlayerId{22});
		require(!wrongPlayer.accepted, "claimed wrong player rejected");
		require(wrongPlayer.reason == BackendRejectReason::InvalidOwnership, "wrong player reason");

		const auto ticked = harness.manager.tick(harness.match);
		require(ticked.accepted, "match tick accepted");
		harness.registry.find(harness.blue)->flushOutbound();
		harness.registry.find(harness.red)->flushOutbound();
		require(!harness.blueOutbound.sent.empty(), "blue received snapshot/events");
		require(!harness.redOutbound.sent.empty(), "red received snapshot/events");
		bool sawObstacles = false;
		bool sawCrow = false;
		for (const auto& payload : harness.blueOutbound.sent)
		{
			if (payload.find("\"obstacles\":[") != std::string::npos && payload.find("\"x\":7") != std::string::npos)
			{
				sawObstacles = true;
			}
			if (payload.find("\"kind\":\"crow\"") != std::string::npos)
			{
				sawCrow = true;
			}
		}
		require(sawObstacles, "snapshot payload broadcasts authoritative obstacle cells");
		require(sawCrow, "snapshot payload broadcasts neutral crow hazard");
		require(std::any_of(harness.blueOutbound.sent.begin(), harness.blueOutbound.sent.end(), [](const std::string& payload) {
			        return payload.find("\"id\":\"tower_left\"") != std::string::npos &&
			               payload.find("\"radius\":0.8") != std::string::npos &&
			               payload.find("\"range\":2.2") != std::string::npos &&
			               payload.find("\"effect\":\"damage_drop_objective\"") != std::string::npos &&
			               payload.find("\"trigger\":\"range\"") != std::string::npos &&
			               payload.find("\"icon\":\"hazard_tower\"") != std::string::npos &&
			               payload.find("\"cooldownTicks\":20") != std::string::npos;
		        }),
		        "snapshot payload broadcasts hazard metadata");

		const auto metrics = harness.manager.metrics();
		require(metrics.commandsAccepted == 1, "accepted command metric increments");
		require(metrics.commandsApplied == 1, "applied command metric increments");
		require(metrics.commandsRejected == 2, "rejection metric increments");
		require(metrics.snapshotsBroadcast == 2, "snapshot broadcast counted per recipient");
		require(metrics.eventBatchesBroadcast == 2, "event batch broadcast counted per recipient");
	}

	void tickCanAdvanceWithoutSnapshotBroadcast()
	{
		MatchHarness harness;

		const auto first = harness.manager.tick(harness.match, false);
		require(first.accepted, "tick without snapshot broadcast accepted");
		const auto afterFirst = harness.manager.view(harness.match);
		require(afterFirst.has_value() && afterFirst->snapshot.has_value(), "authoritative snapshot remains queryable");
		require(afterFirst->snapshot->tick == 1, "simulation tick advances without snapshot fanout");
		require(harness.manager.metrics().snapshotsBroadcast == 0, "snapshot fanout is suppressed until snapshotRate is due");

		const auto second = harness.manager.tick(harness.match, true);
		require(second.accepted, "snapshot due tick accepted");
		require(harness.manager.metrics().snapshotsBroadcast == 2, "snapshot due tick broadcasts to both participants");
	}

	void completeObjectiveRunMatchWithFakeSessions()
	{
		MatchHarness harness;
		std::uint64_t seq = 1;
		const BackendCommand moveNorth{BackendCommandKind::Move, {0, -1}};
		const BackendCommand moveSouth{BackendCommandKind::Move, {0, 1}};
		const BackendCommand stop{BackendCommandKind::Stop, {}};

		for (int step = 0; step < 14; ++step)
		{
			require(harness.manager.submitCommand(harness.blue, harness.match, seq++, moveNorth).accepted,
			        "blue direct movement toward objective accepted");
			require(harness.manager.tick(harness.match).accepted, "movement tick accepted");
		}

		require(harness.manager.submitCommand(harness.blue, harness.match, seq++, stop).accepted, "stop at objective accepted");
		require(harness.manager.tick(harness.match).accepted, "stop tick accepted");
		auto carryingView = harness.manager.view(harness.match);
		require(carryingView.has_value() && carryingView->snapshot.has_value(), "carrying match view exists");
		require(carryingView->snapshot->objective.state == if_arena::battle_core::ObjectiveState::Carried,
		        "objective pickup is automatic on contact");

		for (int step = 0; step < 24; ++step)
		{
			require(harness.manager.submitCommand(harness.blue, harness.match, seq++, moveSouth).accepted,
			        "blue direct return movement accepted");
			require(harness.manager.tick(harness.match).accepted, "return tick accepted");
			const auto progress = harness.manager.view(harness.match);
			if (progress.has_value() && progress->snapshot.has_value() && progress->snapshot->finished)
			{
				break;
			}
		}

		const auto view = harness.manager.view(harness.match);
		require(view.has_value(), "completed match view exists");
		require(view->snapshot.has_value(), "completed match has snapshot");
		std::string finishMessage = "capture reaches score limit and finishes match";
		if (!view->snapshot->players.empty())
		{
			const auto& player = view->snapshot->players.front();
			finishMessage += " at tick " + std::to_string(view->snapshot->tick) + " player=(" +
			                 std::to_string(player.worldPosition.x) + "," + std::to_string(player.worldPosition.y) +
			                 ") objectiveState=" + std::to_string(static_cast<int>(view->snapshot->objective.state));
		}
		require(view->snapshot->finished, finishMessage);

		bool blueScored = false;
		for (const auto& score : view->snapshot->scores)
		{
			if (score.team == if_arena::battle_core::ArenaTeam::Blue && score.score == 1)
			{
				blueScored = true;
			}
		}
		require(blueScored, "blue score increments after capture");
		require(harness.manager.metrics().snapshotsBroadcast >= 20, "snapshots broadcast during full fake-session match");
	}

	void wrongSessionAndUnstartedMatchCommandsRejected()
	{
		FakeOutboundSession ownerOutbound;
		FakeOutboundSession strangerOutbound;
		SessionRegistry registry;
		MatchManager manager{registry};
		const auto ownerCreated = registry.createSession(ConnectionId{1}, ownerOutbound);
		const auto strangerCreated = registry.createSession(ConnectionId{2}, strangerOutbound);
		require(ownerCreated.session.has_value(), "owner session created");
		require(strangerCreated.session.has_value(), "stranger session created");
		require(registry.find(*ownerCreated.session)->authenticate(PlayerId{1}).accepted, "owner authenticated");
		require(registry.find(*strangerCreated.session)->authenticate(PlayerId{2}).accepted, "stranger authenticated");

		const auto created = manager.createMatch(*ownerCreated.session);
		require(created.accepted(), "waiting match created");
		require(created.match.has_value(), "waiting match id returned");

		const BackendCommand stop{BackendCommandKind::Stop, {}};
		const auto unstarted = manager.submitCommand(*ownerCreated.session, *created.match, 1, stop);
		require(!unstarted.accepted, "command before full match rejected");
		require(unstarted.reason == BackendRejectReason::MatchNotStarted, "unstarted match reason");

		MatchHarness harness;
		FakeOutboundSession thirdOutbound;
		const auto third = harness.registry.createSession(ConnectionId{3}, thirdOutbound);
		require(third.session.has_value(), "third session created");
		require(harness.registry.find(*third.session)->authenticate(PlayerId{33}).accepted, "third session authenticated");
		const auto wrongSession = harness.manager.submitCommand(*third.session, harness.match, 1, stop);
		require(!wrongSession.accepted, "session outside match rejected");
		require(wrongSession.reason == BackendRejectReason::InvalidOwnership, "wrong session rejected before core");
	}

	void commandQueueBoundRejectsSpamBeforeCore()
	{
		BackendLimits limits;
		limits.maxPendingCommandsPerSession = 1;
		limits.maxPendingCommandsPerMatch = 2;
		limits.maxCommandsPerSessionPerTick = 4;

		FakeOutboundSession blueOutbound;
		FakeOutboundSession redOutbound;
		SessionRegistry registry{limits};
		MatchManager manager{registry, limits};
		const auto blueSession = registry.createSession(ConnectionId{1}, blueOutbound);
		const auto redSession = registry.createSession(ConnectionId{2}, redOutbound);
		require(blueSession.session.has_value(), "blue session created");
		require(redSession.session.has_value(), "red session created");
		require(registry.find(*blueSession.session)->authenticate(PlayerId{11}).accepted, "blue authenticated");
		require(registry.find(*redSession.session)->authenticate(PlayerId{22}).accepted, "red authenticated");
		const auto created = manager.createMatch(*blueSession.session);
		require(created.accepted(), "match created");
		const auto joined = manager.joinMatch(*redSession.session, created.joinCode);
		require(joined.accepted(), "match joined");
		require(joined.match.has_value(), "match id returned");

		const BackendCommand moveNorth{BackendCommandKind::Move, {0, -1}};
		require(manager.submitCommand(*blueSession.session, *joined.match, 1, moveNorth).accepted, "first command queued");
		const auto overflow = manager.submitCommand(*blueSession.session, *joined.match, 2, moveNorth);
		require(!overflow.accepted, "second command rejected by per-session queue bound");
		require(overflow.reason == BackendRejectReason::QueueFull, "queue full reason returned for command spam");
		const auto view = manager.view(*joined.match);
		require(view.has_value(), "match view available after queue overflow");
		require(view->pendingCommands == 1, "command queue remains bounded after rejection");
		require(manager.metrics().queueOverflows == 1, "command queue overflow metric increments");
	}

	void spamAndSlowSessionAreBounded()
	{
		BackendLimits limits;
		limits.maxPendingCommandsPerSession = 2;
		limits.maxCommandsPerSessionPerTick = 2;
		limits.maxPendingOutboundMessages = 2;
		limits.maxPendingOutboundBytes = 4096;

		FakeOutboundSession blueOutbound;
		FakeOutboundSession redOutbound;
		SessionRegistry registry{limits};
		MatchManager manager{registry, limits};
		const auto blueSession = registry.createSession(ConnectionId{1}, blueOutbound);
		const auto redSession = registry.createSession(ConnectionId{2}, redOutbound);
		require(blueSession.session.has_value(), "blue session created");
		require(redSession.session.has_value(), "red session created");
		require(registry.find(*blueSession.session)->authenticate(PlayerId{11}).accepted, "blue authenticated");
		require(registry.find(*redSession.session)->authenticate(PlayerId{22}).accepted, "red authenticated");
		const auto created = manager.createMatch(*blueSession.session);
		require(created.accepted(), "match created");
		const auto joined = manager.joinMatch(*redSession.session, created.joinCode);
		require(joined.accepted(), "match joined");
		require(joined.match.has_value(), "match id returned");

		const BackendCommand moveNorth{BackendCommandKind::Move, {0, -1}};
		require(manager.submitCommand(*blueSession.session, *joined.match, 1, moveNorth).accepted, "first command queued");
		require(manager.submitCommand(*blueSession.session, *joined.match, 2, moveNorth).accepted, "second command queued");
		const auto third = manager.submitCommand(*blueSession.session, *joined.match, 3, moveNorth);
		require(!third.accepted, "spam command rejected");
		require(third.reason == BackendRejectReason::RateLimited, "spam rejection is rate limit");

		require(manager.tick(*joined.match).accepted, "first tick");
		registry.find(*redSession.session)->flushOutbound();
		require(manager.tick(*joined.match).accepted, "second tick");
		const auto* slowSession = registry.find(*blueSession.session);
		require(slowSession != nullptr, "slow session still findable");
		require(slowSession->authState() == SessionAuthState::Closed, "slow session closed by bounded outbound queue");
		require(slowSession->pendingOutboundMessages() == 0, "closed slow session has no pending messages");
		require(manager.metrics().disconnects == 1, "disconnect metric increments on queue overflow");
		require(manager.metrics().queueOverflows == 1, "queue overflow metric increments");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"sessionCanBeCreatedAndClosed", sessionCanBeCreatedAndClosed},
		{"commandBeforeAuthRejected", commandBeforeAuthRejected},
		{"outgoingQueueIsBounded", outgoingQueueIsBounded},
		{"scenarioConfigLoadsPlayableArena", scenarioConfigLoadsPlayableArena},
		{"scenarioConfigRejectsInvalidArena", scenarioConfigRejectsInvalidArena},
		{"scenarioConfigRejectsInvalidHazardMetadata", scenarioConfigRejectsInvalidHazardMetadata},
		{"flushSendsQueuedMessages", flushSendsQueuedMessages},
		{"matchStartsThroughJoinCode", matchStartsThroughJoinCode},
		{"commandsAreOwnedSequencedAndBroadcast", commandsAreOwnedSequencedAndBroadcast},
		{"tickCanAdvanceWithoutSnapshotBroadcast", tickCanAdvanceWithoutSnapshotBroadcast},
		{"completeObjectiveRunMatchWithFakeSessions", completeObjectiveRunMatchWithFakeSessions},
		{"wrongSessionAndUnstartedMatchCommandsRejected", wrongSessionAndUnstartedMatchCommandsRejected},
		{"commandQueueBoundRejectsSpamBeforeCore", commandQueueBoundRejectsSpamBeforeCore},
		{"spamAndSlowSessionAreBounded", spamAndSlowSessionAreBounded},
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
