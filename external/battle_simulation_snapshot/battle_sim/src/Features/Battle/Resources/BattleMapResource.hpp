// Battle-owned rectangular map resource with occupancy queries.

#pragma once

#include "Core/CommonTypes.hpp"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

namespace battle_sim::features::battle
{
	class BattleMapResource
	{
	public:
		BattleMapResource(int width, int height);

		int width() const;
		int height() const;
		bool inside(core::Position position) const;
		bool isOccupied(core::Position position) const;
		std::optional<core::EntityId> entityAt(core::Position position) const;

		void place(core::EntityId entity, core::Position position);
		void move(core::EntityId entity, core::Position from, core::Position to);
		void remove(core::EntityId entity, core::Position position);

		static int distance(core::Position lhs, core::Position rhs);
		std::vector<core::EntityId> entitiesInRadius(core::Position center, int radius) const;
		std::vector<core::EntityId> entitiesInDistanceRange(core::Position center, int minDistance, int maxDistance) const;
		std::vector<core::Position> neighbors8(core::Position center) const;

	private:
		int _width{};
		int _height{};
		std::unordered_map<std::uint64_t, core::EntityId> _occupancy;

		std::uint64_t key(core::Position position) const;
		core::Position positionFromKey(std::uint64_t key) const;
	};
}
