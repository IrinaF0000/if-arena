#pragma once

#include "BattleEngine.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace if_arena::battle_core
{
	struct ArenaDimensions
	{
		int width{21};
		int height{13};

		friend constexpr bool operator==(ArenaDimensions, ArenaDimensions) = default;
	};

	struct ArenaCircleZone
	{
		Vec2i center{};
		double radius{};

		friend constexpr bool operator==(ArenaCircleZone, ArenaCircleZone) = default;
	};

	struct ArenaSpawn
	{
		Vec2i cell{};

		friend constexpr bool operator==(ArenaSpawn, ArenaSpawn) = default;
	};

	struct ArenaConfig
	{
		ArenaDimensions dimensions{};
		std::optional<ArenaCircleZone> redBase;
		std::optional<ArenaCircleZone> blueBase;
		std::optional<ArenaSpawn> redSpawn;
		std::optional<ArenaSpawn> blueSpawn;
		std::optional<Vec2i> objectiveSpawn;
		std::vector<Vec2i> obstacles;
		std::vector<HazardConfig> hazards;
	};

	struct ArenaValidationResult
	{
		std::vector<std::string> errors;

		[[nodiscard]] bool valid() const
		{
			return errors.empty();
		}
	};

	inline constexpr ArenaDimensions canonicalObjectiveRunDimensions()
	{
		return ArenaDimensions{21, 13};
	}

	inline constexpr Vec2i rotate180(Vec2i position, ArenaDimensions dimensions)
	{
		return Vec2i{dimensions.width - 1 - position.x, dimensions.height - 1 - position.y};
	}

	inline constexpr Vec2i toPlayerView(Vec2i worldPosition, ArenaTeam viewer, ArenaDimensions dimensions)
	{
		return viewer == ArenaTeam::Red ? rotate180(worldPosition, dimensions) : worldPosition;
	}

	inline constexpr Direction inputDirectionToWorld(Direction localDirection, ArenaTeam viewer)
	{
		return viewer == ArenaTeam::Red ? Direction{-localDirection.dx, -localDirection.dy} : localDirection;
	}

	inline ArenaConfig makeSmallObjectiveRunArenaConfig()
	{
		ArenaConfig config;
		config.dimensions = canonicalObjectiveRunDimensions();
		config.redBase = ArenaCircleZone{Vec2i{10, 1}, 1.5};
		config.blueBase = ArenaCircleZone{Vec2i{10, 11}, 1.5};
		config.redSpawn = ArenaSpawn{Vec2i{10, 2}};
		config.blueSpawn = ArenaSpawn{Vec2i{10, 10}};
		config.objectiveSpawn = Vec2i{10, 6};
		config.obstacles = {
			Vec2i{6, 4},
			Vec2i{7, 4},
			Vec2i{14, 8},
			Vec2i{13, 8},
			Vec2i{14, 4},
			Vec2i{13, 4},
			Vec2i{6, 8},
			Vec2i{7, 8},
			Vec2i{9, 5},
			Vec2i{11, 7},
			Vec2i{11, 5},
			Vec2i{9, 7},
		};
		config.hazards = {
			HazardConfig{HazardKind::Tower, Vec2i{5, 6}, 0.8, 2.2, 6, 20},
			HazardConfig{HazardKind::Tower, Vec2i{15, 6}, 0.8, 2.2, 6, 20},
			HazardConfig{HazardKind::Mine, Vec2i{8, 6}, 0.7, 1.0, 12, 30},
			HazardConfig{HazardKind::Mine, Vec2i{12, 6}, 0.7, 1.0, 12, 30},
		};
		return config;
	}

	inline ArenaValidationResult validateArenaConfig(const ArenaConfig& config)
	{
		ArenaValidationResult result;
		const auto addError = [&result](std::string message) {
			result.errors.push_back(std::move(message));
		};
		const auto inBounds = [&config](Vec2i position) {
			return position.x >= 0 && position.y >= 0 && position.x < config.dimensions.width &&
			       position.y < config.dimensions.height;
		};
		const auto hasObstacle = [&config](Vec2i cell) {
			return std::find(config.obstacles.begin(), config.obstacles.end(), cell) != config.obstacles.end();
		};
		const auto validateZone = [&](const std::optional<ArenaCircleZone>& zone, const char* name) {
			if (!zone.has_value())
			{
				addError(std::string{name} + " is required");
				return;
			}
			if (!inBounds(zone->center))
			{
				addError(std::string{name} + " center must be inside arena bounds");
			}
			if (!std::isfinite(zone->radius) || zone->radius <= 0.0)
			{
				addError(std::string{name} + " radius must be positive");
			}
		};
		const auto validateSpawn = [&](const std::optional<ArenaSpawn>& spawn, const char* name) {
			if (!spawn.has_value())
			{
				addError(std::string{name} + " is required");
				return;
			}
			if (!inBounds(spawn->cell))
			{
				addError(std::string{name} + " must be inside arena bounds");
			}
			if (hasObstacle(spawn->cell))
			{
				addError(std::string{name} + " must not be blocked by an obstacle");
			}
		};

		const bool dimensionsPositive = config.dimensions.width > 0 && config.dimensions.height > 0;
		if (!dimensionsPositive)
		{
			addError("arena dimensions must be positive");
		}
		else if (config.dimensions != canonicalObjectiveRunDimensions())
		{
			addError("objective run arena dimensions must be 21x13");
		}

		validateZone(config.redBase, "red base");
		validateZone(config.blueBase, "blue base");
		validateSpawn(config.redSpawn, "red spawn");
		validateSpawn(config.blueSpawn, "blue spawn");

		if (!config.objectiveSpawn.has_value())
		{
			addError("objective spawn is required");
		}
		else
		{
			if (!inBounds(*config.objectiveSpawn))
			{
				addError("objective spawn must be inside arena bounds");
			}
			if (rotate180(*config.objectiveSpawn, config.dimensions) != *config.objectiveSpawn)
			{
				addError("objective spawn must be at the rotational center");
			}
			if (hasObstacle(*config.objectiveSpawn))
			{
				addError("objective spawn must not be blocked by an obstacle");
			}
		}

		if (config.redBase.has_value() && config.blueBase.has_value())
		{
			if (rotate180(config.redBase->center, config.dimensions) != config.blueBase->center)
			{
				addError("bases must use 180-degree rotational symmetry");
			}
			if (config.redBase->radius != config.blueBase->radius)
			{
				addError("base radii must match");
			}
		}

		if (config.redSpawn.has_value() && config.blueSpawn.has_value() &&
		    rotate180(config.redSpawn->cell, config.dimensions) != config.blueSpawn->cell)
		{
			addError("spawns must use 180-degree rotational symmetry");
		}

		if (dimensionsPositive &&
		    config.obstacles.size() > static_cast<std::size_t>(config.dimensions.width * config.dimensions.height))
		{
			addError("obstacle count exceeds arena cell count");
		}
		for (std::size_t index = 0; index < config.obstacles.size(); ++index)
		{
			const Vec2i obstacle = config.obstacles[index];
			if (!inBounds(obstacle))
			{
				addError("obstacle must be inside arena bounds");
				continue;
			}
			if (std::find(config.obstacles.begin() + static_cast<std::ptrdiff_t>(index) + 1, config.obstacles.end(),
			              obstacle) != config.obstacles.end())
			{
				addError("obstacle cells must be unique");
			}
			if (!hasObstacle(rotate180(obstacle, config.dimensions)))
			{
				addError("obstacles must use 180-degree rotational symmetry");
			}
		}
		for (std::size_t index = 0; index < config.hazards.size(); ++index)
		{
			const auto& hazard = config.hazards[index];
			if (!inBounds(hazard.position))
			{
				addError("hazard must be inside arena bounds");
				continue;
			}
			if (hasObstacle(hazard.position))
			{
				addError("hazard must not be inside an obstacle");
			}
			if (config.objectiveSpawn.has_value() && hazard.position == *config.objectiveSpawn)
			{
				addError("hazard must not overlap the objective spawn");
			}
			if ((config.redSpawn.has_value() && hazard.position == config.redSpawn->cell) ||
			    (config.blueSpawn.has_value() && hazard.position == config.blueSpawn->cell))
			{
				addError("hazard must not overlap a player spawn");
			}
			const Vec2i mirrored = rotate180(hazard.position, config.dimensions);
			const bool hasMirror = std::any_of(config.hazards.begin(), config.hazards.end(), [&](const HazardConfig& other) {
				return other.kind == hazard.kind && other.position == mirrored && other.damage == hazard.damage &&
				       other.cooldownTicks == hazard.cooldownTicks && other.radius == hazard.radius && other.range == hazard.range;
			});
			if (!hasMirror)
			{
				addError("hazards must use 180-degree rotational symmetry");
			}
		}

		return result;
	}
}
