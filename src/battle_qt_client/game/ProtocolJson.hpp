#pragma once

#include "ClientTypes.hpp"

#include <QJsonObject>
#include <QString>

#include <optional>

namespace if_arena::battle_qt_client::game
{
	struct ParseFailure
	{
		QString message;
	};

	template <typename T>
	struct ParseResult
	{
		std::optional<T> value;
		std::optional<ParseFailure> error;

		[[nodiscard]] bool ok() const
		{
			return value.has_value();
		}
	};

	[[nodiscard]] ParseResult<ArenaSnapshot> parseArenaSnapshotPayload(const QString& payloadJson);
	[[nodiscard]] ParseResult<MatchJoined> parseMatchJoinedPayload(const QString& payloadJson);
	[[nodiscard]] ParseResult<QString> parseSessionIdPayload(const QString& payloadJson);
	[[nodiscard]] ParseResult<QString> parseErrorMessagePayload(const QString& payloadJson);
	[[nodiscard]] QJsonObject intentPayload(const QString& matchId, const ClientIntent& intent);
}
