// App-level JSON trace inspection.

#include "App/TraceInspector.hpp"

#include <algorithm>
#include <cctype>
#include <istream>
#include <iterator>
#include <ostream>
#include <regex>
#include <stdexcept>
#include <string>

namespace battle_sim::app
{
	namespace
	{
		struct TraceSummary
		{
			int ticks{};
			int unitsSpawned{};
			int moves{};
			int attacks{};
			int deaths{};
		};

		std::string readAll(std::istream& input)
		{
			return std::string(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
		}

		std::string trim(std::string text)
		{
			const auto first = std::find_if_not(
				text.begin(),
				text.end(),
				[](unsigned char ch) { return std::isspace(ch) != 0; });
			const auto last = std::find_if_not(
				text.rbegin(),
				text.rend(),
				[](unsigned char ch) { return std::isspace(ch) != 0; }).base();
			if (first >= last)
			{
				return {};
			}
			return std::string(first, last);
		}

		TraceSummary summarizeTrace(const std::string& text)
		{
			const auto trimmed = trim(text);
			if (trimmed.size() < 2 || trimmed.front() != '[' || trimmed.back() != ']')
			{
				throw std::runtime_error("Malformed trace: expected a JSON array");
			}

			TraceSummary summary;
			const std::regex tickPattern("\"tick\"\\s*:\\s*([0-9]+)");
			for (std::sregex_iterator it(trimmed.begin(), trimmed.end(), tickPattern), end; it != end; ++it)
			{
				summary.ticks = std::max(summary.ticks, std::stoi((*it)[1].str()));
			}

			const std::regex eventPattern("\"event\"\\s*:\\s*\"([^\"]+)\"");
			for (std::sregex_iterator it(trimmed.begin(), trimmed.end(), eventPattern), end; it != end; ++it)
			{
				const auto eventName = (*it)[1].str();
				if (eventName == "UNIT_SPAWNED")
				{
					++summary.unitsSpawned;
				}
				else if (eventName == "UNIT_MOVED")
				{
					++summary.moves;
				}
				else if (eventName == "UNIT_ATTACKED")
				{
					++summary.attacks;
				}
				else if (eventName == "UNIT_DIED")
				{
					++summary.deaths;
				}
			}

			return summary;
		}
	}

	void inspectTrace(std::istream& input, std::ostream& output)
	{
		const auto summary = summarizeTrace(readAll(input));
		output << "Battle summary\n";
		output << "Ticks: " << summary.ticks << '\n';
		output << "Units spawned: " << summary.unitsSpawned << '\n';
		output << "Moves: " << summary.moves << '\n';
		output << "Attacks: " << summary.attacks << '\n';
		output << "Deaths: " << summary.deaths << '\n';
	}
}
