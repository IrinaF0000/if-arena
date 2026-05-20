#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace if_arena::battle_core
{
	struct PlayerId
	{
		std::uint32_t value{};

		friend constexpr bool operator==(PlayerId, PlayerId) = default;
	};

	struct EntityId
	{
		std::uint32_t value{};

		friend constexpr bool operator==(EntityId, EntityId) = default;
	};

	struct Vec2i
	{
		int x{};
		int y{};

		friend constexpr bool operator==(Vec2i, Vec2i) = default;
	};

	struct Vec2d
	{
		double x{};
		double y{};

		friend constexpr bool operator==(Vec2d, Vec2d) = default;
	};

	struct MovementVector
	{
		double dx{};
		double dy{};

		friend constexpr bool operator==(MovementVector, MovementVector) = default;
	};

	enum class ArenaTeam
	{
		Red,
		Blue
	};

	struct BaseZoneConfig
	{
		ArenaTeam team{ArenaTeam::Blue};
		Vec2i center{};
		double radius{1.5};
	};

	enum class ObjectiveState
	{
		AtSpawn,
		Dropped,
		Carried,
		Respawning,
		Captured
	};

	struct ObjectiveConfig
	{
		Vec2i spawn{10, 6};
		double pickupRadius{0.75};
		double carrierSpeedMultiplier{0.8};
		std::uint32_t pickupLockTicks{10};
		std::uint32_t captureRespawnDelayTicks{20};
		std::uint32_t scoreLimit{3};
	};

	struct PlayerConfig
	{
		PlayerId player{};
		ArenaTeam team{ArenaTeam::Blue};
		Vec2i spawn{};
		int heroHp{100};
	};

	struct MatchConfig
	{
		int width{21};
		int height{13};
		std::uint32_t maxTicks{1200};
		double playerSpeedPerTick{1.0};
		double playerCollisionRadius{0.35};
		std::vector<PlayerConfig> players;
		std::vector<Vec2i> obstacles;
		std::vector<BaseZoneConfig> bases;
		std::optional<ObjectiveConfig> objective;
	};

	struct Direction
	{
		int dx{};
		int dy{};
	};

	enum class PlayerCommandType
	{
		Move,
		Stop,
		Attack,
		Interact
	};

	struct PlayerCommand
	{
		PlayerId player{};
		PlayerCommandType type{PlayerCommandType::Stop};
		Direction direction{};

		static PlayerCommand move(PlayerId player, Direction direction);
		static PlayerCommand stop(PlayerId player);
	};

	enum class CommandStatus
	{
		Accepted,
		Rejected
	};

	struct CommandResult
	{
		CommandStatus status{CommandStatus::Rejected};
		std::string reason;

		[[nodiscard]] bool accepted() const;
	};

	struct PlayerSnapshot
	{
		PlayerId player{};
		EntityId hero{};
		ArenaTeam team{ArenaTeam::Blue};
		Vec2i spawn{};
		Vec2i position{};
		Vec2d worldPosition{};
		MovementVector desiredMovement{};
		int hp{};
		bool inOwnBase{};
	};

	struct ObjectiveSnapshot
	{
		ObjectiveState state{ObjectiveState::AtSpawn};
		Vec2d position{};
		PlayerId carrier{};
		std::uint32_t pickupLockTicksRemaining{};
		std::uint32_t respawnTicksRemaining{};
	};

	struct ScoreSnapshot
	{
		ArenaTeam team{ArenaTeam::Blue};
		std::uint32_t score{};
	};

	struct BattleSnapshot
	{
		std::uint32_t tick{};
		int width{};
		int height{};
		bool finished{};
		std::vector<PlayerSnapshot> players;
		ObjectiveSnapshot objective;
		std::vector<ScoreSnapshot> scores;
	};

	enum class BattleEventType
	{
		PlayerMoved,
		TickAdvanced,
		MatchFinished,
		ObjectivePickedUp,
		ObjectiveDropped,
		ObjectiveCaptured,
		ObjectiveRespawned,
		ScoreChanged
	};

	struct BattleEvent
	{
		BattleEventType type{BattleEventType::TickAdvanced};
		std::uint32_t tick{};
		PlayerId player{};
		Vec2i from{};
		Vec2i to{};
		ArenaTeam team{ArenaTeam::Blue};
		std::uint32_t score{};
	};

	class BattleEngine
	{
	public:
		explicit BattleEngine(MatchConfig config);

		CommandResult submit(PlayerCommand command);
		CommandResult dropObjective(PlayerId carrier);
		std::vector<BattleEvent> tick();

		[[nodiscard]] BattleSnapshot snapshot() const;

	private:
		struct PendingCommand
		{
			PlayerCommand command;
		};

		std::uint32_t _tick{};
		int _width{};
		int _height{};
		std::uint32_t _maxTicks{};
		double _playerSpeedPerTick{};
		double _playerCollisionRadius{};
		bool _finished{};
		std::vector<PlayerSnapshot> _players;
		std::vector<Vec2i> _obstacles;
		std::vector<BaseZoneConfig> _bases;
		std::vector<PendingCommand> _pendingCommands;
		std::vector<BattleEvent> _systemEvents;
		std::optional<ObjectiveConfig> _objectiveConfig;
		ObjectiveSnapshot _objective;
		std::vector<ScoreSnapshot> _scores;

		[[nodiscard]] PlayerSnapshot* findPlayer(PlayerId player);
		[[nodiscard]] const PlayerSnapshot* findPlayer(PlayerId player) const;
		[[nodiscard]] ScoreSnapshot* findScore(ArenaTeam team);
		[[nodiscard]] bool inBounds(Vec2i position) const;
		[[nodiscard]] bool inBounds(Vec2d position) const;
		[[nodiscard]] bool obstacleAt(Vec2i cell) const;
		[[nodiscard]] bool collidesWithObstacle(Vec2d from, Vec2d to) const;
		[[nodiscard]] bool isInOwnBase(const PlayerSnapshot& player) const;
		[[nodiscard]] bool canPickupObjective(const PlayerSnapshot& player) const;
		void pickUpObjective(PlayerSnapshot& player, std::vector<BattleEvent>& events);
		void captureObjective(PlayerSnapshot& player, std::vector<BattleEvent>& events);
		void updateObjectiveTimers(std::vector<BattleEvent>& events);
	};
}
