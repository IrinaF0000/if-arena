#include "game/CoordinateTransform.hpp"

#include <iostream>
#include <string>

namespace
{
	using if_arena::battle_qt_client::game::Direction;
	using if_arena::battle_qt_client::game::MapSize;
	using if_arena::battle_qt_client::game::Team;
	using if_arena::battle_qt_client::game::WorldPoint;
	using if_arena::battle_qt_client::game::clampDirection;
	using if_arena::battle_qt_client::game::localDirectionToCanonical;
	using if_arena::battle_qt_client::game::localDirectionToServerCommand;
	using if_arena::battle_qt_client::game::worldToPlayerView;

	int failures = 0;

	void expect(bool condition, const std::string& message)
	{
		if (!condition)
		{
			std::cerr << "FAIL: " << message << '\n';
			++failures;
		}
	}

	bool same(WorldPoint lhs, WorldPoint rhs)
	{
		return lhs.x == rhs.x && lhs.y == rhs.y;
	}

	bool same(Direction lhs, Direction rhs)
	{
		return lhs.dx == rhs.dx && lhs.dy == rhs.dy;
	}
}

int main()
{
	const MapSize map{21.0, 13.0};

	expect(same(worldToPlayerView({2.0, 10.0}, map, Team::Blue), {2.0, 10.0}),
	       "blue view keeps canonical coordinates");
	expect(same(worldToPlayerView({2.0, 10.0}, map, Team::Red), {18.0, 2.0}),
	       "red view rotates coordinates so own base is at bottom");
	expect(same(worldToPlayerView({10.0, 6.0}, map, Team::Red), {10.0, 6.0}),
	       "center objective stays centered for red view");
	expect(same(worldToPlayerView({3.0, 4.0}, map, Team::Red), {17.0, 8.0}),
	       "hazard positions transform without changing authority");

	expect(same(localDirectionToCanonical({0, -1}, Team::Blue), {0, -1}), "blue forward is canonical up");
	expect(same(localDirectionToCanonical({0, -1}, Team::Red), {0, 1}), "red forward is canonical down");
	expect(same(localDirectionToCanonical({1, 0}, Team::Blue), {1, 0}), "blue right stays canonical right");
	expect(same(localDirectionToCanonical({1, 0}, Team::Red), {-1, 0}), "red right rotates to canonical left");
	expect(same(localDirectionToServerCommand({0, -1}), {0, -1}),
	       "server command keeps team-local intent for current backend contract");
	expect(same(localDirectionToServerCommand({1, 0}), {1, 0}), "server command keeps local lateral intent");
	expect(same(clampDirection(4, -3), {1, -1}), "directions are clamped to unit steps");

	if (failures != 0)
	{
		return 1;
	}
	std::cout << "CoordinateTransformTests passed\n";
	return 0;
}
