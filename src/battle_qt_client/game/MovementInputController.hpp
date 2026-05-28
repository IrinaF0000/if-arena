#pragma once

#include "CoordinateTransform.hpp"

#include <cstdint>
#include <optional>

namespace if_arena::battle_qt_client::game
{
	enum class MovementInputCommandKind
	{
		Move,
		Stop
	};

	struct MovementInputCommand
	{
		MovementInputCommandKind kind{MovementInputCommandKind::Stop};
		Direction direction{};
	};

	class MovementInputController
	{
	public:
		explicit MovementInputController(std::int64_t resendIntervalMs = 150);

		[[nodiscard]] std::optional<MovementInputCommand> updateDesired(Direction direction, std::int64_t nowMs);
		[[nodiscard]] std::optional<MovementInputCommand> poll(std::int64_t nowMs);
		void markRejected();
		void reset();

		[[nodiscard]] Direction desiredDirection() const;
		[[nodiscard]] std::int64_t resendIntervalMs() const;

	private:
		[[nodiscard]] bool shouldSendMove(Direction direction, std::int64_t nowMs) const;
		void recordSent(Direction direction, std::int64_t nowMs);

		Direction _desired{};
		std::optional<Direction> _lastSent;
		std::int64_t _lastSentAtMs{};
		std::int64_t _resendIntervalMs{150};
	};
}
