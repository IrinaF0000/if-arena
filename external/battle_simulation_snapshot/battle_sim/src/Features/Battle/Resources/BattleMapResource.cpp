// Battle-owned rectangular map resource with occupancy queries.

#include "Features/Battle/Resources/BattleMapResource.hpp"

#include <algorithm>
#include <cstdlib>
#include <stdexcept>

namespace battle_sim::features::battle
{
	BattleMapResource::BattleMapResource(int width, int height)
		: _width(width)
		, _height(height)
	{
		if (width <= 0 || height <= 0)
		{
			throw std::runtime_error("Battle map size must be positive");
		}
	}

	int BattleMapResource::width() const
	{
		return _width;
	}

	int BattleMapResource::height() const
	{
		return _height;
	}

	bool BattleMapResource::inside(core::Position position) const
	{
		return position.x >= 0 && position.y >= 0 && position.x < _width && position.y < _height;
	}

	bool BattleMapResource::isOccupied(core::Position position) const
	{
		return _occupancy.contains(key(position));
	}

	std::optional<core::EntityId> BattleMapResource::entityAt(core::Position position) const
	{
		const auto it = _occupancy.find(key(position));
		if (it == _occupancy.end())
		{
			return std::nullopt;
		}
		return it->second;
	}

	void BattleMapResource::place(core::EntityId entity, core::Position position)
	{
		_occupancy.emplace(key(position), entity);
	}

	void BattleMapResource::move(core::EntityId entity, core::Position from, core::Position to)
	{
		remove(entity, from);
		place(entity, to);
	}

	void BattleMapResource::remove(core::EntityId entity, core::Position position)
	{
		const auto it = _occupancy.find(key(position));
		if (it != _occupancy.end() && it->second == entity)
		{
			_occupancy.erase(it);
		}
	}

	int BattleMapResource::distance(core::Position lhs, core::Position rhs)
	{
		return std::max(std::abs(lhs.x - rhs.x), std::abs(lhs.y - rhs.y));
	}

	std::vector<core::EntityId> BattleMapResource::entitiesInRadius(core::Position center, int radius) const
	{
		std::vector<core::EntityId> entities;
		entities.reserve(_occupancy.size());
		for (const auto& [cell, entity] : _occupancy)
		{
			if (distance(center, positionFromKey(cell)) <= radius)
			{
				entities.push_back(entity);
			}
		}
		return entities;
	}

	std::vector<core::EntityId> BattleMapResource::entitiesInDistanceRange(
		core::Position center,
		int minDistance,
		int maxDistance) const
	{
		std::vector<core::EntityId> entities;
		entities.reserve(_occupancy.size());
		for (const auto& [cell, entity] : _occupancy)
		{
			const int dist = distance(center, positionFromKey(cell));
			if (dist >= minDistance && dist <= maxDistance)
			{
				entities.push_back(entity);
			}
		}
		return entities;
	}

	std::vector<core::Position> BattleMapResource::neighbors8(core::Position center) const
	{
		std::vector<core::Position> result;
		result.reserve(8);
		for (int dy = -1; dy <= 1; ++dy)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				if (dx == 0 && dy == 0)
				{
					continue;
				}

				core::Position position{center.x + dx, center.y + dy};
				if (inside(position))
				{
					result.push_back(position);
				}
			}
		}
		return result;
	}

	std::uint64_t BattleMapResource::key(core::Position position) const
	{
		return static_cast<std::uint64_t>(position.y) * static_cast<std::uint64_t>(_width)
			+ static_cast<std::uint64_t>(position.x);
	}

	core::Position BattleMapResource::positionFromKey(std::uint64_t cell) const
	{
		return core::Position{
			static_cast<int>(cell % static_cast<std::uint64_t>(_width)),
			static_cast<int>(cell / static_cast<std::uint64_t>(_width))};
	}
}
