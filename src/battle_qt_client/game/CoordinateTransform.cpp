#include "CoordinateTransform.hpp"

namespace if_arena::battle_qt_client::game
{
	WorldPoint worldToPlayerView(WorldPoint point, MapSize map, Team viewer)
	{
		if (viewer != Team::Red)
		{
			return point;
		}
		return WorldPoint{std::max(0.0, map.width - 1.0 - point.x), std::max(0.0, map.height - 1.0 - point.y)};
	}

	Direction localDirectionToCanonical(Direction direction, Team viewer)
	{
		if (viewer != Team::Red)
		{
			return direction;
		}
		return Direction{-direction.dx, -direction.dy};
	}

	Direction localDirectionToServerCommand(Direction direction)
	{
		return direction;
	}

	Direction clampDirection(int dx, int dy)
	{
		return Direction{std::clamp(dx, -1, 1), std::clamp(dy, -1, 1)};
	}
}
