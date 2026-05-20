#include "BattleEngine.hpp"
#include "ArenaConfig.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_core;

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	MatchConfig onePlayerMatch()
	{
		MatchConfig config;
		config.width = 3;
		config.height = 3;
		config.maxTicks = 3;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 1}, 100});
		return config;
	}

	MatchConfig objectiveRunMatch()
	{
		const auto arena = makeSmallObjectiveRunArenaConfig();
		MatchConfig config;
		config.width = arena.dimensions.width;
		config.height = arena.dimensions.height;
		config.obstacles = arena.obstacles;
		config.bases = {
			BaseZoneConfig{ArenaTeam::Red, arena.redBase->center, arena.redBase->radius},
			BaseZoneConfig{ArenaTeam::Blue, arena.blueBase->center, arena.blueBase->radius},
		};
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Red, arena.redSpawn->cell, 100});
		config.players.push_back(PlayerConfig{PlayerId{2}, ArenaTeam::Blue, arena.blueSpawn->cell, 100});
		return config;
	}

	void createsMatchSnapshot()
	{
		BattleEngine engine(onePlayerMatch());
		const auto snapshot = engine.snapshot();

		require(snapshot.tick == 0, "new match starts at tick zero");
		require(snapshot.width == 3, "snapshot exposes match width");
		require(snapshot.height == 3, "snapshot exposes match height");
		require(!snapshot.finished, "new match is not finished");
		require(snapshot.players.size() == 1, "new match has one configured player");
		require(snapshot.players.front().player == PlayerId{1}, "snapshot preserves player id");
		require(snapshot.players.front().position == Vec2i{1, 1}, "snapshot preserves spawn");
		require(snapshot.players.front().worldPosition == Vec2d{1.0, 1.0}, "snapshot exposes smooth world position");
		require(snapshot.players.front().team == ArenaTeam::Blue, "snapshot exposes player team");
	}

	void appliesDeterministicMoveIntent()
	{
		BattleEngine first(onePlayerMatch());
		BattleEngine second(onePlayerMatch());

		const auto firstResult = first.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		const auto secondResult = second.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(firstResult.accepted(), "first move command accepted");
		require(secondResult.accepted(), "second move command accepted");

		const auto firstEvents = first.tick();
		const auto secondEvents = second.tick();
		const auto firstSnapshot = first.snapshot();
		const auto secondSnapshot = second.snapshot();

		require(firstEvents.size() == secondEvents.size(), "event count is deterministic");
		require(firstEvents.front().type == BattleEventType::PlayerMoved, "first event is movement");
		require(firstEvents.front().from == Vec2i{1, 1}, "movement event records source");
		require(firstEvents.front().to == Vec2i{2, 1}, "movement event records destination");
		require(firstSnapshot.tick == secondSnapshot.tick, "tick is deterministic");
		require(firstSnapshot.players.front().position == secondSnapshot.players.front().position, "position is deterministic");
		require(firstSnapshot.players.front().position == Vec2i{2, 1}, "move intent updates position by one cell");
	}

	void rejectsInvalidClientAuthority()
	{
		BattleEngine engine(onePlayerMatch());

		const auto unknownPlayer = engine.submit(PlayerCommand::move(PlayerId{99}, Direction{1, 0}));
		require(!unknownPlayer.accepted(), "unknown player command rejected");

		const auto oversizedMove = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{2, 0}));
		require(!oversizedMove.accepted(), "oversized movement intent rejected");

		const auto unsupportedAttack = engine.submit(PlayerCommand{PlayerId{1}, PlayerCommandType::Attack, {}});
		require(!unsupportedAttack.accepted(), "unimplemented attack command rejected");

		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();
		require(events.size() == 1, "invalid commands do not emit gameplay events");
		require(snapshot.players.front().position == Vec2i{1, 1}, "invalid commands do not mutate position");
	}

	void movesSmoothlyUntilStopped()
	{
		auto config = onePlayerMatch();
		config.playerSpeedPerTick = 0.5;
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(move.accepted(), "smooth move command accepted");
		engine.tick();
		const auto moving = engine.snapshot();
		require(moving.players.front().worldPosition == Vec2d{1.5, 1.0}, "smooth movement advances by configured speed");
		require(moving.players.front().desiredMovement == MovementVector{1.0, 0.0}, "move intent becomes desired movement");

		const auto stop = engine.submit(PlayerCommand::stop(PlayerId{1}));
		require(stop.accepted(), "stop command accepted");
		engine.tick();
		const auto stopped = engine.snapshot();
		require(stopped.players.front().worldPosition == Vec2d{1.5, 1.0}, "stop prevents further movement");
		require(stopped.players.front().desiredMovement == MovementVector{}, "stop clears desired movement");
	}

	void preventsLeavingArenaBounds()
	{
		MatchConfig config;
		config.width = 3;
		config.height = 3;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{0, 1}, 100});
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{-1, 0}));
		require(move.accepted(), "boundary move intent accepted for server-side resolution");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{0, 1}, "player cannot leave arena bounds");
		require(snapshot.players.front().worldPosition == Vec2d{0.0, 1.0}, "blocked boundary move does not change world position");
	}

	void preventsMovingThroughObstacles()
	{
		MatchConfig config;
		config.width = 5;
		config.height = 3;
		config.playerSpeedPerTick = 2.0;
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{1, 1}, 100});
		config.obstacles.push_back(Vec2i{2, 1});
		BattleEngine engine(config);

		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(move.accepted(), "obstacle move intent accepted for server-side resolution");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{1, 1}, "player cannot move through obstacle");
		require(snapshot.players.front().worldPosition == Vec2d{1.0, 1.0}, "blocked obstacle move does not change world position");
	}

	void reportsBaseEntry()
	{
		MatchConfig config;
		config.width = 21;
		config.height = 13;
		config.bases.push_back(BaseZoneConfig{ArenaTeam::Blue, Vec2i{10, 11}, 0.5});
		config.players.push_back(PlayerConfig{PlayerId{1}, ArenaTeam::Blue, Vec2i{10, 10}, 100});
		BattleEngine engine(config);

		require(!engine.snapshot().players.front().inOwnBase, "player starts outside own base zone");
		const auto move = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{0, 1}));
		require(move.accepted(), "base entry movement accepted");
		engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.players.front().position == Vec2i{10, 11}, "player reaches own base cell");
		require(snapshot.players.front().inOwnBase, "player is marked inside own base");
	}

	void transformsPlayerOrientedCoordinates()
	{
		const auto dimensions = canonicalObjectiveRunDimensions();

		require(toPlayerView(Vec2i{10, 10}, ArenaTeam::Blue, dimensions) == Vec2i{10, 10},
		        "blue view keeps canonical bottom spawn");
		require(toPlayerView(Vec2i{10, 2}, ArenaTeam::Red, dimensions) == Vec2i{10, 10},
		        "red view rotates own top spawn to bottom");
		require(inputDirectionToWorld(Direction{0, -1}, ArenaTeam::Blue).dy == -1,
		        "blue forward input maps upward in canonical coordinates");
		require(inputDirectionToWorld(Direction{0, -1}, ArenaTeam::Red).dy == 1,
		        "red forward input maps downward in canonical coordinates");
	}

	void finishesAtConfiguredTickLimit()
	{
		BattleEngine engine(onePlayerMatch());

		engine.tick();
		engine.tick();
		const auto events = engine.tick();
		const auto snapshot = engine.snapshot();

		require(snapshot.finished, "match finishes at max tick");
		require(events.back().type == BattleEventType::MatchFinished, "finish event is emitted");

		const auto afterFinish = engine.submit(PlayerCommand::move(PlayerId{1}, Direction{1, 0}));
		require(!afterFinish.accepted(), "finished match rejects commands");
	}

	void acceptsCanonicalObjectiveRunArena()
	{
		const auto arena = makeSmallObjectiveRunArenaConfig();
		const auto validation = validateArenaConfig(arena);

		require(validation.valid(), "canonical 21x13 Objective Run arena is valid");
		require(arena.dimensions == canonicalObjectiveRunDimensions(), "canonical arena dimensions are 21x13");
		require(arena.objectiveSpawn == Vec2i{10, 6}, "canonical objective starts at center");
		require(arena.redBase.has_value() && arena.blueBase.has_value(), "canonical arena has both bases");
		require(arena.redSpawn.has_value() && arena.blueSpawn.has_value(), "canonical arena has both spawns");
	}

	void createsMatchFromCanonicalObjectiveRunArena()
	{
		BattleEngine engine(objectiveRunMatch());
		const auto snapshot = engine.snapshot();

		require(snapshot.width == 21, "canonical arena match exposes width");
		require(snapshot.height == 13, "canonical arena match exposes height");
		require(snapshot.players.size() == 2, "canonical arena match has both players");
		require(snapshot.players.front().team == ArenaTeam::Red, "red player keeps team assignment");
		require(snapshot.players.back().team == ArenaTeam::Blue, "blue player keeps team assignment");
		require(snapshot.players.front().spawn == Vec2i{10, 2}, "red spawn comes from canonical arena");
		require(snapshot.players.back().spawn == Vec2i{10, 10}, "blue spawn comes from canonical arena");
	}

	void rejectsAsymmetricObstacleLayout()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.obstacles.push_back(Vec2i{1, 1});

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "asymmetric obstacle layout is rejected");
	}

	void rejectsInvalidArenaDimensions()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.dimensions = ArenaDimensions{20, 13};

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "non-canonical arena dimensions are rejected");
	}

	void rejectsOutOfBoundsArenaObjects()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.redSpawn = ArenaSpawn{Vec2i{-1, 2}};
		arena.obstacles.push_back(Vec2i{21, 6});

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "out-of-bounds arena objects are rejected");
	}

	void rejectsMissingBases()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.redBase.reset();

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "missing base is rejected");
	}

	void rejectsInvalidObjectiveSpawn()
	{
		auto arena = makeSmallObjectiveRunArenaConfig();
		arena.objectiveSpawn = Vec2i{9, 6};

		const auto validation = validateArenaConfig(arena);

		require(!validation.valid(), "non-central objective spawn is rejected");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"createsMatchSnapshot", createsMatchSnapshot},
		{"appliesDeterministicMoveIntent", appliesDeterministicMoveIntent},
		{"rejectsInvalidClientAuthority", rejectsInvalidClientAuthority},
		{"movesSmoothlyUntilStopped", movesSmoothlyUntilStopped},
		{"preventsLeavingArenaBounds", preventsLeavingArenaBounds},
		{"preventsMovingThroughObstacles", preventsMovingThroughObstacles},
		{"reportsBaseEntry", reportsBaseEntry},
		{"transformsPlayerOrientedCoordinates", transformsPlayerOrientedCoordinates},
		{"finishesAtConfiguredTickLimit", finishesAtConfiguredTickLimit},
		{"acceptsCanonicalObjectiveRunArena", acceptsCanonicalObjectiveRunArena},
		{"createsMatchFromCanonicalObjectiveRunArena", createsMatchFromCanonicalObjectiveRunArena},
		{"rejectsAsymmetricObstacleLayout", rejectsAsymmetricObstacleLayout},
		{"rejectsInvalidArenaDimensions", rejectsInvalidArenaDimensions},
		{"rejectsOutOfBoundsArenaObjects", rejectsOutOfBoundsArenaObjects},
		{"rejectsMissingBases", rejectsMissingBases},
		{"rejectsInvalidObjectiveSpawn", rejectsInvalidObjectiveSpawn},
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
