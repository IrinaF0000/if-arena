#include "ScenarioConfig.hpp"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

namespace if_arena::battle_backend
{
	namespace
	{
		bool isSpace(char value)
		{
			return value == ' ' || value == '\n' || value == '\r' || value == '\t';
		}

		std::size_t skipWhitespace(std::string_view text, std::size_t offset)
		{
			while (offset < text.size() && isSpace(text[offset]))
			{
				++offset;
			}
			return offset;
		}

		std::string trimCopy(std::string_view text)
		{
			std::size_t begin = 0;
			while (begin < text.size() && isSpace(text[begin]))
			{
				++begin;
			}
			std::size_t end = text.size();
			while (end > begin && isSpace(text[end - 1]))
			{
				--end;
			}
			return std::string{text.substr(begin, end - begin)};
		}

		std::optional<std::size_t> findStringEnd(std::string_view text, std::size_t quote)
		{
			bool escaped = false;
			for (std::size_t index = quote + 1; index < text.size(); ++index)
			{
				if (escaped)
				{
					escaped = false;
					continue;
				}
				if (text[index] == '\\')
				{
					escaped = true;
					continue;
				}
				if (text[index] == '"')
				{
					return index;
				}
			}
			return std::nullopt;
		}

		std::optional<std::size_t> findBalancedEnd(std::string_view text, std::size_t openAt, char open, char close)
		{
			int depth = 0;
			for (std::size_t index = openAt; index < text.size(); ++index)
			{
				if (text[index] == '"')
				{
					const auto stringEnd = findStringEnd(text, index);
					if (!stringEnd.has_value())
					{
						return std::nullopt;
					}
					index = *stringEnd;
					continue;
				}
				if (text[index] == open)
				{
					++depth;
				}
				if (text[index] == close)
				{
					--depth;
					if (depth == 0)
					{
						return index;
					}
				}
			}
			return std::nullopt;
		}

		std::optional<std::size_t> findKey(std::string_view object, std::string_view key)
		{
			const std::string needle = "\"" + std::string{key} + "\"";
			std::size_t searchFrom = 0;
			while (searchFrom < object.size())
			{
				const auto found = object.find(needle, searchFrom);
				if (found == std::string_view::npos)
				{
					return std::nullopt;
				}
				const auto afterKey = skipWhitespace(object, found + needle.size());
				if (afterKey < object.size() && object[afterKey] == ':')
				{
					return skipWhitespace(object, afterKey + 1);
				}
				searchFrom = found + needle.size();
			}
			return std::nullopt;
		}

		std::optional<std::string_view> objectField(std::string_view object, std::string_view key)
		{
			const auto valueStart = findKey(object, key);
			if (!valueStart.has_value() || *valueStart >= object.size() || object[*valueStart] != '{')
			{
				return std::nullopt;
			}
			const auto valueEnd = findBalancedEnd(object, *valueStart, '{', '}');
			if (!valueEnd.has_value())
			{
				return std::nullopt;
			}
			return object.substr(*valueStart, *valueEnd - *valueStart + 1);
		}

		std::optional<std::string_view> arrayField(std::string_view object, std::string_view key)
		{
			const auto valueStart = findKey(object, key);
			if (!valueStart.has_value() || *valueStart >= object.size() || object[*valueStart] != '[')
			{
				return std::nullopt;
			}
			const auto valueEnd = findBalancedEnd(object, *valueStart, '[', ']');
			if (!valueEnd.has_value())
			{
				return std::nullopt;
			}
			return object.substr(*valueStart, *valueEnd - *valueStart + 1);
		}

		std::optional<std::string_view> rawField(std::string_view object, std::string_view key)
		{
			const auto valueStart = findKey(object, key);
			if (!valueStart.has_value())
			{
				return std::nullopt;
			}
			if (*valueStart < object.size() && object[*valueStart] == '"')
			{
				const auto valueEnd = findStringEnd(object, *valueStart);
				if (!valueEnd.has_value())
				{
					return std::nullopt;
				}
				return object.substr(*valueStart, *valueEnd - *valueStart + 1);
			}
			if (*valueStart < object.size() && object[*valueStart] == '{')
			{
				const auto valueEnd = findBalancedEnd(object, *valueStart, '{', '}');
				if (!valueEnd.has_value())
				{
					return std::nullopt;
				}
				return object.substr(*valueStart, *valueEnd - *valueStart + 1);
			}
			if (*valueStart < object.size() && object[*valueStart] == '[')
			{
				const auto valueEnd = findBalancedEnd(object, *valueStart, '[', ']');
				if (!valueEnd.has_value())
				{
					return std::nullopt;
				}
				return object.substr(*valueStart, *valueEnd - *valueStart + 1);
			}
			std::size_t valueEnd = *valueStart;
			while (valueEnd < object.size() && object[valueEnd] != ',' && object[valueEnd] != '}')
			{
				++valueEnd;
			}
			return object.substr(*valueStart, valueEnd - *valueStart);
		}

		std::optional<std::string> stringField(std::string_view object, std::string_view key)
		{
			const auto raw = rawField(object, key);
			if (!raw.has_value() || raw->size() < 2 || raw->front() != '"' || raw->back() != '"')
			{
				return std::nullopt;
			}
			std::string output;
			bool escaped = false;
			for (std::size_t index = 1; index + 1 < raw->size(); ++index)
			{
				const char ch = (*raw)[index];
				if (escaped)
				{
					output.push_back(ch);
					escaped = false;
					continue;
				}
				if (ch == '\\')
				{
					escaped = true;
					continue;
				}
				output.push_back(ch);
			}
			return output;
		}

		std::optional<double> numberField(std::string_view object, std::string_view key)
		{
			const auto raw = rawField(object, key);
			if (!raw.has_value())
			{
				return std::nullopt;
			}
			const auto valueText = trimCopy(*raw);
			double value = 0.0;
			const auto* begin = valueText.data();
			const auto* end = begin + valueText.size();
			const auto parsed = std::from_chars(begin, end, value);
			if (parsed.ec != std::errc{} || parsed.ptr != end || !std::isfinite(value))
			{
				return std::nullopt;
			}
			return value;
		}

		std::optional<std::int64_t> intField(std::string_view object, std::string_view key)
		{
			const auto raw = rawField(object, key);
			if (!raw.has_value())
			{
				return std::nullopt;
			}
			const auto valueText = trimCopy(*raw);
			std::int64_t value = 0;
			const auto* begin = valueText.data();
			const auto* end = begin + valueText.size();
			const auto parsed = std::from_chars(begin, end, value);
			if (parsed.ec != std::errc{} || parsed.ptr != end)
			{
				return std::nullopt;
			}
			return value;
		}

		std::optional<bool> boolField(std::string_view object, std::string_view key)
		{
			const auto raw = rawField(object, key);
			if (!raw.has_value())
			{
				return std::nullopt;
			}
			const auto value = trimCopy(*raw);
			if (value == "true")
			{
				return true;
			}
			if (value == "false")
			{
				return false;
			}
			return std::nullopt;
		}

		std::vector<std::string_view> objectArrayItems(std::string_view array)
		{
			std::vector<std::string_view> result;
			std::size_t index = skipWhitespace(array, 0);
			if (index >= array.size() || array[index] != '[')
			{
				return result;
			}
			++index;
			while (index < array.size())
			{
				index = skipWhitespace(array, index);
				if (index < array.size() && array[index] == ']')
				{
					break;
				}
				if (index >= array.size() || array[index] != '{')
				{
					break;
				}
				const auto end = findBalancedEnd(array, index, '{', '}');
				if (!end.has_value())
				{
					break;
				}
				result.push_back(array.substr(index, *end - index + 1));
				index = skipWhitespace(array, *end + 1);
				if (index < array.size() && array[index] == ',')
				{
					++index;
				}
			}
			return result;
		}

		void addError(std::vector<std::string>& errors, std::string message)
		{
			errors.push_back(std::move(message));
		}

		std::optional<battle_core::Vec2i> pointFromObject(std::string_view object)
		{
			const auto x = intField(object, "x");
			const auto y = intField(object, "y");
			if (!x.has_value() || !y.has_value() || *x < std::numeric_limits<int>::min() ||
			    *x > std::numeric_limits<int>::max() || *y < std::numeric_limits<int>::min() ||
			    *y > std::numeric_limits<int>::max())
			{
				return std::nullopt;
			}
			return battle_core::Vec2i{static_cast<int>(*x), static_cast<int>(*y)};
		}

		std::optional<battle_core::Vec2i> pointField(std::string_view object, std::string_view key)
		{
			const auto point = objectField(object, key);
			if (!point.has_value())
			{
				return std::nullopt;
			}
			return pointFromObject(*point);
		}

		std::uint32_t msToTicks(double milliseconds, std::uint32_t tickRate)
		{
			const double ticks = milliseconds * static_cast<double>(tickRate) / 1000.0;
			return static_cast<std::uint32_t>(std::max(1.0, std::round(ticks)));
		}

		std::optional<battle_core::HazardKind> hazardKindFrom(std::string_view kind)
		{
			if (kind == "mine")
			{
				return battle_core::HazardKind::Mine;
			}
			if (kind == "tower")
			{
				return battle_core::HazardKind::Tower;
			}
			if (kind == "crow")
			{
				return battle_core::HazardKind::Crow;
			}
			return std::nullopt;
		}

		std::optional<battle_core::HazardEffect> hazardEffectFrom(std::string_view effect)
		{
			if (effect == "damage")
			{
				return battle_core::HazardEffect::Damage;
			}
			if (effect == "damage_drop_objective")
			{
				return battle_core::HazardEffect::DamageAndDropObjective;
			}
			return std::nullopt;
		}

		std::optional<battle_core::HazardTrigger> hazardTriggerFrom(std::string_view trigger)
		{
			if (trigger == "proximity")
			{
				return battle_core::HazardTrigger::Proximity;
			}
			if (trigger == "range")
			{
				return battle_core::HazardTrigger::Range;
			}
			return std::nullopt;
		}

		bool isSafeIdentifier(std::string_view text)
		{
			if (text.empty() || text.size() > 64)
			{
				return false;
			}
			return std::all_of(text.begin(), text.end(), [](char ch) {
				return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
				       ch == '_' || ch == '-';
			});
		}

		std::optional<battle_core::HazardConfig> parseHazard(std::string_view object, std::uint32_t tickRate,
		                                                     std::vector<std::string>& errors)
		{
			if (const auto enabled = boolField(object, "enabled"); enabled.has_value() && !*enabled)
			{
				return std::nullopt;
			}
			const auto id = stringField(object, "id");
			const auto kindText = stringField(object, "kind");
			const auto position = pointFromObject(object);
			const auto radius = numberField(object, "radius");
			const auto range = numberField(object, "range");
			const auto damage = intField(object, "damage");
			const auto effectText = stringField(object, "effect");
			const auto triggerText = stringField(object, "trigger");
			const auto icon = stringField(object, "icon");
			const auto cooldownMs = numberField(object, "cooldownMs");
			const auto cooldownTicks = intField(object, "cooldownTicks");
			const auto patrolRadius = numberField(object, "patrolRadius");
			if (!id.has_value() || !kindText.has_value() || !position.has_value() || !radius.has_value() ||
			    !range.has_value() || !damage.has_value() || !effectText.has_value() || !triggerText.has_value() ||
			    !icon.has_value() || (!cooldownMs.has_value() && !cooldownTicks.has_value()))
			{
				addError(errors, "hazard must include id, kind, x, y, radius, range, damage, effect, trigger, icon, and cooldown");
				return std::nullopt;
			}
			if (!isSafeIdentifier(*id))
			{
				addError(errors, "hazard id must be a safe identifier");
				return std::nullopt;
			}
			if (!isSafeIdentifier(*icon))
			{
				addError(errors, "hazard icon must be a safe identifier");
				return std::nullopt;
			}
			const auto kind = hazardKindFrom(*kindText);
			if (!kind.has_value())
			{
				addError(errors, "unsupported hazard kind: " + *kindText);
				return std::nullopt;
			}
			const auto effect = hazardEffectFrom(*effectText);
			if (!effect.has_value())
			{
				addError(errors, "unsupported hazard effect: " + *effectText);
				return std::nullopt;
			}
			const auto trigger = hazardTriggerFrom(*triggerText);
			if (!trigger.has_value())
			{
				addError(errors, "unsupported hazard trigger: " + *triggerText);
				return std::nullopt;
			}
			if (*damage <= 0 || *damage > std::numeric_limits<int>::max())
			{
				addError(errors, "hazard damage must be positive");
				return std::nullopt;
			}
			std::uint32_t cooldown = 0;
			if (cooldownTicks.has_value())
			{
				if (*cooldownTicks <= 0 || *cooldownTicks > std::numeric_limits<std::uint32_t>::max())
				{
					addError(errors, "hazard cooldownTicks must be positive");
					return std::nullopt;
				}
				cooldown = static_cast<std::uint32_t>(*cooldownTicks);
			}
			else
			{
				cooldown = msToTicks(*cooldownMs, tickRate);
			}
			const auto seed = intField(object, "seed").value_or(0);
			if (seed < 0 || seed > std::numeric_limits<std::uint32_t>::max())
			{
				addError(errors, "hazard seed must be a uint32");
				return std::nullopt;
			}
			if (patrolRadius.has_value() &&
			    (!std::isfinite(*patrolRadius) ||
			     (*kind == battle_core::HazardKind::Crow && *patrolRadius < 1.0) ||
			     (*kind != battle_core::HazardKind::Crow && *patrolRadius <= 0.0)))
			{
				addError(errors, "hazard patrolRadius must be positive and allow crow movement");
				return std::nullopt;
			}
			return battle_core::HazardConfig{*kind,
			                                 *position,
			                                 *radius,
			                                 *range,
			                                 static_cast<int>(*damage),
			                                 cooldown,
			                                 static_cast<std::uint32_t>(seed),
			                                 *id,
			                                 *effect,
			                                 *trigger,
			                                 *icon,
			                                 patrolRadius.value_or(battle_core::HazardConfig{}.patrolRadius)};
		}
	}

	PlayableScenarioConfig makeFallbackObjectiveRunScenario()
	{
		const auto arena = battle_core::makeSmallObjectiveRunArenaConfig();
		PlayableScenarioConfig scenario;
		scenario.arena = arena;
		scenario.objective = battle_core::ObjectiveConfig{*arena.objectiveSpawn, 0.75, 0.8, 10, 20, 1};
		scenario.combat = battle_core::CombatConfig{25, 1.25, 3, 2.0, 5};
		return scenario;
	}

	ScenarioLoadResult parseScenarioConfig(std::string_view json)
	{
		ScenarioLoadResult result;
		PlayableScenarioConfig scenario;
		scenario.id = stringField(json, "scenario").value_or("");
		scenario.mode = stringField(json, "mode").value_or("");
		if (scenario.id != "arena_small_objective_run")
		{
			addError(result.errors, "unsupported scenario id");
		}
		if (scenario.mode != "objective_run")
		{
			addError(result.errors, "unsupported scenario mode");
		}

		const auto rules = objectField(json, "rules");
		const std::uint32_t tickRate = [&]() {
			if (!rules.has_value())
			{
				addError(result.errors, "rules object is required");
				return 20u;
			}
			const auto value = intField(*rules, "tickRate");
			if (!value.has_value() || *value <= 0 || *value > 240)
			{
				addError(result.errors, "rules.tickRate must be in range 1..240");
				return 20u;
			}
			return static_cast<std::uint32_t>(*value);
		}();

		const auto map = objectField(json, "map");
		if (!map.has_value())
		{
			addError(result.errors, "map object is required");
		}
		else
		{
			const auto width = intField(*map, "width");
			const auto height = intField(*map, "height");
			if (!width.has_value() || !height.has_value() || *width <= 0 || *height <= 0 ||
			    *width > std::numeric_limits<int>::max() || *height > std::numeric_limits<int>::max())
			{
				addError(result.errors, "map width and height must be positive integers");
			}
			else
			{
				scenario.arena.dimensions =
				    battle_core::ArenaDimensions{static_cast<int>(*width), static_cast<int>(*height)};
			}
			const auto symmetry = stringField(*map, "symmetry");
			if (symmetry.value_or("") != "rotational_180")
			{
				addError(result.errors, "map.symmetry must be rotational_180");
			}
			const auto redBase = objectField(*map, "redBase");
			const auto blueBase = objectField(*map, "blueBase");
			if (!redBase.has_value() || !blueBase.has_value())
			{
				addError(result.errors, "redBase and blueBase are required");
			}
			else
			{
				const auto redCenter = pointFromObject(*redBase);
				const auto blueCenter = pointFromObject(*blueBase);
				const auto redRadius = numberField(*redBase, "radius");
				const auto blueRadius = numberField(*blueBase, "radius");
				if (!redCenter.has_value() || !blueCenter.has_value() || !redRadius.has_value() || !blueRadius.has_value())
				{
					addError(result.errors, "base center and radius fields are required");
				}
				else
				{
					scenario.arena.redBase = battle_core::ArenaCircleZone{*redCenter, *redRadius};
					scenario.arena.blueBase = battle_core::ArenaCircleZone{*blueCenter, *blueRadius};
				}
			}
			scenario.arena.objectiveSpawn = pointField(*map, "objectiveSpawn");
			if (!scenario.arena.objectiveSpawn.has_value())
			{
				addError(result.errors, "map.objectiveSpawn is required");
			}
			const auto obstacles = arrayField(*map, "obstacles");
			if (!obstacles.has_value())
			{
				addError(result.errors, "map.obstacles array is required");
			}
			else
			{
				for (const auto obstacle : objectArrayItems(*obstacles))
				{
					const auto point = pointFromObject(obstacle);
					if (!point.has_value())
					{
						addError(result.errors, "obstacle x/y fields are required");
						continue;
					}
					scenario.arena.obstacles.push_back(*point);
				}
			}
		}

		const auto players = objectField(json, "players");
		if (!players.has_value())
		{
			addError(result.errors, "players object is required");
		}
		else
		{
			const auto redSpawn = pointField(*players, "redSpawn");
			const auto blueSpawn = pointField(*players, "blueSpawn");
			if (!redSpawn.has_value() || !blueSpawn.has_value())
			{
				addError(result.errors, "player spawn fields are required");
			}
			else
			{
				scenario.arena.redSpawn = battle_core::ArenaSpawn{*redSpawn};
				scenario.arena.blueSpawn = battle_core::ArenaSpawn{*blueSpawn};
			}
			const auto speed = numberField(*players, "speedPerTick");
			if (!speed.has_value())
			{
				addError(result.errors, "players.speedPerTick is required");
			}
			else
			{
				scenario.playerSpeedPerTick = *speed;
			}
			if (const auto hp = intField(*players, "heroHp"); hp.has_value())
			{
				if (*hp <= 0 || *hp > std::numeric_limits<int>::max())
				{
					addError(result.errors, "players.heroHp must be positive");
				}
				else
				{
					scenario.heroHp = static_cast<int>(*hp);
				}
			}
		}

		const auto objective = objectField(json, "objective");
		if (!objective.has_value())
		{
			addError(result.errors, "objective object is required");
		}
		else
		{
			const auto scoreLimit = intField(*objective, "scoreLimit");
			const auto speedMultiplier = numberField(*objective, "carrierSpeedMultiplier");
			const auto pickupLockMs = numberField(*objective, "pickupLockMs");
			const auto respawnMs = numberField(*objective, "captureRespawnDelayMs");
			const auto pickupRadius = numberField(*objective, "pickupRadius");
			if (!scoreLimit.has_value() || !speedMultiplier.has_value() || !pickupLockMs.has_value() ||
			    !respawnMs.has_value() || !pickupRadius.has_value() || !scenario.arena.objectiveSpawn.has_value())
			{
				addError(result.errors, "objective score, speed, timing, and radius fields are required");
			}
			else if (*scoreLimit <= 0 || *scoreLimit > std::numeric_limits<std::uint32_t>::max())
			{
				addError(result.errors, "objective.scoreLimit must be positive");
			}
			else
			{
				scenario.objective = battle_core::ObjectiveConfig{
					*scenario.arena.objectiveSpawn,
					*pickupRadius,
					*speedMultiplier,
					msToTicks(*pickupLockMs, tickRate),
					msToTicks(*respawnMs, tickRate),
					static_cast<std::uint32_t>(*scoreLimit),
				};
			}
		}

		const auto combat = objectField(json, "combat");
		if (!combat.has_value())
		{
			addError(result.errors, "combat object is required");
		}
		else
		{
			const auto attackDamage = intField(*combat, "attackDamage");
			const auto attackRange = numberField(*combat, "attackRange");
			const auto attackCooldownTicks = intField(*combat, "attackCooldownTicks");
			const auto dashDistance = numberField(*combat, "dashDistance");
			const auto dashCooldownTicks = intField(*combat, "dashCooldownTicks");
			if (!attackDamage.has_value() || !attackRange.has_value() || !attackCooldownTicks.has_value() ||
			    !dashDistance.has_value() || !dashCooldownTicks.has_value())
			{
				addError(result.errors, "combat fields are required");
			}
			else if (*attackDamage <= 0 || *attackCooldownTicks <= 0 || *dashCooldownTicks <= 0)
			{
				addError(result.errors, "combat damage and cooldowns must be positive");
			}
			else
			{
				scenario.combat = battle_core::CombatConfig{static_cast<int>(*attackDamage), *attackRange,
				                                            static_cast<std::uint32_t>(*attackCooldownTicks),
				                                            *dashDistance,
				                                            static_cast<std::uint32_t>(*dashCooldownTicks)};
			}
		}

		const auto hazards = arrayField(json, "hazards");
		if (!hazards.has_value())
		{
			addError(result.errors, "hazards array is required");
		}
		else
		{
			for (const auto hazard : objectArrayItems(*hazards))
			{
				const auto parsed = parseHazard(hazard, tickRate, result.errors);
				if (parsed.has_value())
				{
					scenario.arena.hazards.push_back(*parsed);
				}
			}
		}

		if (rules.has_value())
		{
			const auto timeLimitSeconds = intField(*rules, "timeLimitSeconds");
			if (!timeLimitSeconds.has_value() || *timeLimitSeconds <= 0 ||
			    *timeLimitSeconds > std::numeric_limits<std::uint32_t>::max() / tickRate)
			{
				addError(result.errors, "rules.timeLimitSeconds must be positive");
			}
			else
			{
				scenario.maxTicks = static_cast<std::uint32_t>(*timeLimitSeconds) * tickRate;
			}
		}

		const auto arenaValidation = battle_core::validateArenaConfig(scenario.arena);
		for (const auto& error : arenaValidation.errors)
		{
			addError(result.errors, "arena validation: " + error);
		}
		if (scenario.playerSpeedPerTick <= 0.0)
		{
			addError(result.errors, "players.speedPerTick must be positive");
		}
		if (scenario.maxTicks == 0)
		{
			addError(result.errors, "scenario maxTicks must be positive");
		}
		if (result.errors.empty())
		{
			result.scenario = std::move(scenario);
		}
		return result;
	}
}
