#pragma once

#include "Session.hpp"
#include "BattleEngine.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace if_arena::battle_backend
{
	struct MatchId
	{
		std::uint64_t value{};

		friend constexpr bool operator==(MatchId, MatchId) = default;
	};

	enum class BackendCommandKind
	{
		Move,
		Stop,
		Attack,
		Interact,
		Dash
	};

	struct BackendCommand
	{
		BackendCommandKind kind{BackendCommandKind::Stop};
		battle_core::Direction direction{};
	};

	struct CreateMatchResult
	{
		BackendResult result;
		std::optional<MatchId> match;
		std::string joinCode;

		[[nodiscard]] bool accepted() const;
	};

	struct JoinMatchResult
	{
		BackendResult result;
		std::optional<MatchId> match;
		std::optional<PlayerId> player;

		[[nodiscard]] bool accepted() const;
	};

	struct MatchMetrics
	{
		std::uint64_t activeMatches{};
		std::uint64_t matchesCreated{};
		std::uint64_t commandsAccepted{};
		std::uint64_t commandsRejected{};
		std::uint64_t commandsApplied{};
		std::uint64_t snapshotsBroadcast{};
		std::uint64_t eventBatchesBroadcast{};
		std::uint64_t disconnects{};
		std::uint64_t queueOverflows{};
	};

	struct MatchView
	{
		MatchId match{};
		std::string joinCode;
		bool started{};
		std::size_t playerCount{};
		std::size_t pendingCommands{};
		std::optional<battle_core::BattleSnapshot> snapshot;
	};

	class MatchManager
	{
	public:
		MatchManager(SessionRegistry& sessions, BackendLimits limits = {});

		CreateMatchResult createMatch(SessionId owner);
		JoinMatchResult joinMatch(SessionId session, std::string_view joinCode);
		BackendResult submitCommand(SessionId session, MatchId match, std::uint64_t sessionSeq,
		                            BackendCommand command, std::optional<PlayerId> claimedPlayer = std::nullopt);
		BackendResult tick(MatchId match, bool broadcastSnapshot = true);
		BackendResult disconnect(SessionId session, DisconnectReason reason);

		[[nodiscard]] std::optional<MatchView> view(MatchId match) const;
		[[nodiscard]] MatchMetrics metrics() const;

	private:
		struct Participant
		{
			SessionId session{};
			PlayerId player{};
			battle_core::PlayerId corePlayer{};
			battle_core::ArenaTeam team{battle_core::ArenaTeam::Blue};
			std::uint64_t lastSessionSeq{};
			std::size_t commandsThisTick{};
		};

		struct QueuedCommand
		{
			SessionId session{};
			battle_core::PlayerCommand command{};
		};

		struct MatchRecord
		{
			MatchId id{};
			std::string joinCode;
			std::vector<Participant> participants;
			std::vector<QueuedCommand> queuedCommands;
			std::optional<battle_core::BattleEngine> engine;
		};

		[[nodiscard]] MatchRecord* findMatch(MatchId match);
		[[nodiscard]] const MatchRecord* findMatch(MatchId match) const;
		[[nodiscard]] MatchRecord* findMatchByJoinCode(std::string_view joinCode);
		[[nodiscard]] const MatchRecord* findMatchBySession(SessionId session) const;
		[[nodiscard]] Participant* findParticipant(MatchRecord& match, SessionId session);
		[[nodiscard]] const Participant* findParticipant(const MatchRecord& match, SessionId session) const;
		[[nodiscard]] std::size_t pendingForSession(const MatchRecord& match, SessionId session) const;
		[[nodiscard]] battle_core::MatchConfig makeMatchConfig(const MatchRecord& match) const;
		[[nodiscard]] std::optional<battle_core::PlayerCommand> toCoreCommand(const Participant& participant,
		                                                                      BackendCommand command) const;
		BackendResult startMatchIfReady(MatchRecord& match);
		void broadcast(MatchRecord& match, std::string payload, bool countAsSnapshot);

		SessionRegistry& _sessions;
		BackendLimits _limits;
		MatchMetrics _metrics;
		std::uint64_t _nextMatchId{1};
		std::vector<MatchRecord> _matches;
	};
}
