#pragma once

#include <algorithm>

namespace if_arena::battle_qt_client::game
{
	enum class Team
	{
		Unknown,
		Blue,
		Red
	};

	struct MapSize
	{
		double width{};
		double height{};
	};

	struct WorldPoint
	{
		double x{};
		double y{};
	};

	struct Direction
	{
		int dx{};
		int dy{};
	};

	[[nodiscard]] WorldPoint worldToPlayerView(WorldPoint point, MapSize map, Team viewer);
	[[nodiscard]] Direction localDirectionToCanonical(Direction direction, Team viewer);
	[[nodiscard]] Direction localDirectionToServerCommand(Direction direction);
	[[nodiscard]] Direction clampDirection(int dx, int dy);
}
