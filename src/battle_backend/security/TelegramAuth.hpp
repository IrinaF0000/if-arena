#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace if_arena::battle_backend::security
{
	enum class TelegramAuthErrorCode
	{
		None,
		MissingBotToken,
		OversizedInitData,
		MalformedInitData,
		MissingHash,
		MissingAuthDate,
		InvalidAuthDate,
		StaleAuthDate,
		InvalidHash
	};

	struct TelegramAuthConfig
	{
		std::string botToken;
		std::uint64_t nowUnixSeconds{};
		std::uint64_t maxAgeSeconds{86400};
		std::size_t maxInitDataBytes{4096};
	};

	struct TelegramAuthData
	{
		std::uint64_t authDate{};
		std::string queryId;
		std::string userJson;
	};

	struct TelegramAuthResult
	{
		std::optional<TelegramAuthData> data;
		TelegramAuthErrorCode error{TelegramAuthErrorCode::None};

		[[nodiscard]] bool ok() const;
	};

	class TelegramAuthValidator
	{
	public:
		explicit TelegramAuthValidator(TelegramAuthConfig config);

		[[nodiscard]] TelegramAuthResult validate(std::string_view initData) const;

	private:
		TelegramAuthConfig _config;
	};

	[[nodiscard]] std::string redactTelegramInitDataForLog(std::string_view initData);
}
