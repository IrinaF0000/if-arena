#include "security/TelegramAuth.hpp"

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
	using namespace if_arena::battle_backend::security;

	constexpr const char* ValidInitData =
		"auth_date=1700000000&query_id=test-query&user=%7B%22id%22%3A42%2C%22first_name%22%3A%22Ada%22%7D&hash=950e1d52a70ffd9c49da43fcdfcb0bbde569d0668f91f0d2862ec8efd4d372ff";

	TelegramAuthValidator validator(std::uint64_t now = 1700000100)
	{
		return TelegramAuthValidator(TelegramAuthConfig{
			"TEST_BOT_TOKEN_PLACEHOLDER",
			now,
			300,
			4096,
		});
	}

	void require(bool condition, const std::string& message)
	{
		if (!condition)
		{
			throw std::runtime_error(message);
		}
	}

	void validFixturePasses()
	{
		const auto result = validator().validate(ValidInitData);

		require(result.ok(), "valid Telegram initData passes");
		require(result.data->authDate == 1700000000u, "auth_date parsed");
		require(result.data->queryId == "test-query", "query_id parsed");
		require(result.data->userJson.find("\"id\":42") != std::string::npos, "user payload preserved after validation");
	}

	void invalidHashFails()
	{
		std::string invalid = ValidInitData;
		invalid.replace(invalid.size() - 1u, 1u, "0");

		const auto result = validator().validate(invalid);

		require(!result.ok(), "invalid hash fails");
		require(result.error == TelegramAuthErrorCode::InvalidHash, "invalid hash error");
	}

	void staleAuthDateFails()
	{
		const auto result = validator(1700001001).validate(ValidInitData);

		require(!result.ok(), "stale auth_date fails");
		require(result.error == TelegramAuthErrorCode::StaleAuthDate, "stale error");
	}

	void oversizedInitDataFails()
	{
		TelegramAuthValidator smallLimit(TelegramAuthConfig{"TEST_BOT_TOKEN_PLACEHOLDER", 1700000100, 300, 8});

		const auto result = smallLimit.validate(ValidInitData);

		require(!result.ok(), "oversized initData fails");
		require(result.error == TelegramAuthErrorCode::OversizedInitData, "oversized error");
	}

	void redactionDoesNotLeakRawInitData()
	{
		const auto redacted = redactTelegramInitDataForLog(ValidInitData);

		require(redacted.find("950e1d52") == std::string::npos, "hash not leaked");
		require(redacted.find("test-query") == std::string::npos, "query id not leaked");
		require(redacted == "<redacted:telegram_init_data>", "generic redaction text");
	}
}

int main()
{
	const std::vector<std::pair<const char*, void (*)()>> tests{
		{"validFixturePasses", validFixturePasses},
		{"invalidHashFails", invalidHashFails},
		{"staleAuthDateFails", staleAuthDateFails},
		{"oversizedInitDataFails", oversizedInitDataFails},
		{"redactionDoesNotLeakRawInitData", redactionDoesNotLeakRawInitData},
	};

	int failed = 0;
	for (const auto& [name, test] : tests)
	{
		try
		{
			test();
			std::cout << "[PASS] " << name << '\n';
		}
		catch (const std::exception& ex)
		{
			++failed;
			std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
		}
	}

	if (failed != 0)
	{
		std::cerr << failed << " test(s) failed\n";
		return 1;
	}

	std::cout << tests.size() << " test(s) passed\n";
	return 0;
}
