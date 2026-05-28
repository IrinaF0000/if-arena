#include "game/MovementInputController.hpp"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	using if_arena::battle_qt_client::game::Direction;
	using if_arena::battle_qt_client::game::MovementInputCommand;
	using if_arena::battle_qt_client::game::MovementInputCommandKind;
	using if_arena::battle_qt_client::game::MovementInputController;

	struct ScenarioStep
	{
		std::string action;
		Direction direction{};
		std::int64_t atMs{};
		std::string expect;
	};

	struct Scenario
	{
		std::int64_t resendIntervalMs{};
		std::vector<ScenarioStep> steps;
	};

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	std::string readRepoFile(const std::filesystem::path& relative)
	{
		for (const auto& root : {std::filesystem::path{"."}, std::filesystem::path{".."}})
		{
			const auto path = root / relative;
			std::ifstream input(path);
			if (input)
			{
				std::ostringstream buffer;
				buffer << input.rdbuf();
				return buffer.str();
			}
		}
		throw std::runtime_error("unable to read " + relative.string());
	}

	std::optional<std::string> stringField(std::string_view object, std::string_view key)
	{
		const std::string needle = "\"" + std::string{key} + "\"";
		const auto found = object.find(needle);
		if (found == std::string_view::npos)
		{
			return std::nullopt;
		}
		const auto colon = object.find(':', found + needle.size());
		const auto quote = object.find('"', colon + 1);
		if (colon == std::string_view::npos || quote == std::string_view::npos)
		{
			return std::nullopt;
		}
		const auto end = object.find('"', quote + 1);
		if (end == std::string_view::npos)
		{
			return std::nullopt;
		}
		return std::string{object.substr(quote + 1, end - quote - 1)};
	}

	std::optional<std::int64_t> intField(std::string_view object, std::string_view key)
	{
		const std::string needle = "\"" + std::string{key} + "\"";
		const auto found = object.find(needle);
		if (found == std::string_view::npos)
		{
			return std::nullopt;
		}
		const auto colon = object.find(':', found + needle.size());
		if (colon == std::string_view::npos)
		{
			return std::nullopt;
		}
		auto begin = object.find_first_of("-0123456789", colon + 1);
		if (begin == std::string_view::npos)
		{
			return std::nullopt;
		}
		auto end = begin;
		while (end < object.size() && (object[end] == '-' || (object[end] >= '0' && object[end] <= '9')))
		{
			++end;
		}
		return std::stoll(std::string{object.substr(begin, end - begin)});
	}

	std::vector<std::string_view> stepObjects(std::string_view json)
	{
		std::vector<std::string_view> result;
		const auto stepsKey = json.find("\"steps\"");
		const auto arrayBegin = json.find('[', stepsKey);
		const auto arrayEnd = json.rfind(']');
		require(stepsKey != std::string_view::npos && arrayBegin != std::string_view::npos &&
		            arrayEnd != std::string_view::npos,
		        "scenario steps array exists");
		std::size_t index = arrayBegin + 1;
		while (index < arrayEnd)
		{
			const auto begin = json.find('{', index);
			if (begin == std::string_view::npos || begin > arrayEnd)
			{
				break;
			}
			int depth = 0;
			for (std::size_t cursor = begin; cursor <= arrayEnd; ++cursor)
			{
				if (json[cursor] == '{')
				{
					++depth;
				}
				if (json[cursor] == '}')
				{
					--depth;
					if (depth == 0)
					{
						result.push_back(json.substr(begin, cursor - begin + 1));
						index = cursor + 1;
						break;
					}
				}
			}
		}
		return result;
	}

	Scenario loadScenario()
	{
		const auto json = readRepoFile("tests/scenarios/movement_reliability.json");
		Scenario scenario;
		const auto interval = intField(json, "resendIntervalMs");
		require(interval.has_value() && *interval > 0, "scenario resend interval is positive");
		scenario.resendIntervalMs = *interval;
		for (const auto stepJson : stepObjects(json))
		{
			ScenarioStep step;
			step.action = stringField(stepJson, "action").value_or("");
			step.expect = stringField(stepJson, "expect").value_or("");
			step.atMs = intField(stepJson, "atMs").value_or(-1);
			step.direction = Direction{static_cast<int>(intField(stepJson, "dx").value_or(0)),
			                           static_cast<int>(intField(stepJson, "dy").value_or(0))};
			require(!step.action.empty() && !step.expect.empty() && step.atMs >= 0, "scenario step fields are valid");
			scenario.steps.push_back(step);
		}
		require(!scenario.steps.empty(), "scenario has movement steps");
		return scenario;
	}

	void requireExpected(const std::optional<MovementInputCommand>& command, const ScenarioStep& step)
	{
		if (step.expect == "none")
		{
			require(!command.has_value(), "step expects no command");
			return;
		}
		require(command.has_value(), "step expects a command");
		if (step.expect == "move")
		{
			require(command->kind == MovementInputCommandKind::Move, "step expects move command");
			require(command->direction.dx == step.direction.dx && command->direction.dy == step.direction.dy,
			        "move command uses scenario direction");
			return;
		}
		if (step.expect == "stop")
		{
			require(command->kind == MovementInputCommandKind::Stop, "step expects stop command");
			return;
		}
		throw std::runtime_error("unsupported expected command: " + step.expect);
	}

	void followsConfigDrivenMovementReliabilitySteps()
	{
		const auto scenario = loadScenario();
		MovementInputController controller{scenario.resendIntervalMs};
		for (const auto& step : scenario.steps)
		{
			std::optional<MovementInputCommand> command;
			if (step.action == "setDesired")
			{
				command = controller.updateDesired(step.direction, step.atMs);
			}
			else if (step.action == "poll")
			{
				command = controller.poll(step.atMs);
			}
			else if (step.action == "reject")
			{
				controller.markRejected();
			}
			else
			{
				throw std::runtime_error("unsupported action: " + step.action);
			}
			requireExpected(command, step);
		}
	}
}

int main()
{
	try
	{
		followsConfigDrivenMovementReliabilitySteps();
		std::cout << "[PASS] followsConfigDrivenMovementReliabilitySteps\n";
	}
	catch (const std::exception& ex)
	{
		std::cerr << "[FAIL] " << ex.what() << '\n';
		return 1;
	}
	return 0;
}
