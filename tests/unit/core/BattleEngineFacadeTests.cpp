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
		config.players.push_back(PlayerConfig{PlayerId{1}, Vec2i{1, 1}, 100});
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
		{"finishesAtConfiguredTickLimit", finishesAtConfiguredTickLimit},
		{"acceptsCanonicalObjectiveRunArena", acceptsCanonicalObjectiveRunArena},
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
