// Namespaced identifier for registry handlers.

#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace battle_sim::core::registry
{
	class HandlerId
	{
	public:
		static HandlerId fromString(std::string value)
		{
			return HandlerId(std::move(value));
		}

		explicit HandlerId(std::string value)
			: _value(std::move(value))
		{
			if (!isValid(_value))
			{
				throw std::invalid_argument("Invalid handler id: " + _value);
			}
		}

		std::string_view value() const
		{
			return _value;
		}

		const std::string& str() const
		{
			return _value;
		}

		bool operator==(const HandlerId&) const = default;

	private:
		static bool isValid(std::string_view value)
		{
			if (value.empty() || value.front() == '.' || value.back() == '.')
			{
				return false;
			}

			bool hasSeparator = false;
			for (const char ch : value)
			{
				const bool isLower = ch >= 'a' && ch <= 'z';
				const bool isDigit = ch >= '0' && ch <= '9';
				const bool isSymbol = ch == '.' || ch == '-' || ch == '_';
				if (!isLower && !isDigit && !isSymbol)
				{
					return false;
				}
				hasSeparator = hasSeparator || ch == '.';
			}
			return hasSeparator;
		}

		std::string _value;
	};
}

namespace std
{
	template <>
	struct hash<battle_sim::core::registry::HandlerId>
	{
		std::size_t operator()(const battle_sim::core::registry::HandlerId& id) const noexcept
		{
			return std::hash<std::string>{}(id.str());
		}
	};
}
