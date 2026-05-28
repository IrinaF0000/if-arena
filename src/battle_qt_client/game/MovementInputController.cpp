#include "MovementInputController.hpp"

namespace if_arena::battle_qt_client::game
{
	namespace
	{
		bool isZero(Direction direction)
		{
			return direction.dx == 0 && direction.dy == 0;
		}

		bool sameDirection(Direction lhs, Direction rhs)
		{
			return lhs.dx == rhs.dx && lhs.dy == rhs.dy;
		}
	}

	MovementInputController::MovementInputController(std::int64_t resendIntervalMs)
		: _resendIntervalMs(resendIntervalMs)
	{
	}

	std::optional<MovementInputCommand> MovementInputController::updateDesired(Direction direction, std::int64_t nowMs)
	{
		_desired = direction;
		if (isZero(direction))
		{
			if (!_lastSent.has_value() || isZero(*_lastSent))
			{
				return std::nullopt;
			}
			recordSent(Direction{}, nowMs);
			return MovementInputCommand{MovementInputCommandKind::Stop, {}};
		}
		if (!shouldSendMove(direction, nowMs))
		{
			return std::nullopt;
		}
		recordSent(direction, nowMs);
		return MovementInputCommand{MovementInputCommandKind::Move, direction};
	}

	std::optional<MovementInputCommand> MovementInputController::poll(std::int64_t nowMs)
	{
		if (isZero(_desired) || !shouldSendMove(_desired, nowMs))
		{
			return std::nullopt;
		}
		recordSent(_desired, nowMs);
		return MovementInputCommand{MovementInputCommandKind::Move, _desired};
	}

	void MovementInputController::markRejected()
	{
		_lastSent.reset();
	}

	void MovementInputController::reset()
	{
		_desired = {};
		_lastSent.reset();
		_lastSentAtMs = 0;
	}

	Direction MovementInputController::desiredDirection() const
	{
		return _desired;
	}

	std::int64_t MovementInputController::resendIntervalMs() const
	{
		return _resendIntervalMs;
	}

	bool MovementInputController::shouldSendMove(Direction direction, std::int64_t nowMs) const
	{
		if (!_lastSent.has_value() || !sameDirection(*_lastSent, direction))
		{
			return true;
		}
		return nowMs - _lastSentAtMs >= _resendIntervalMs;
	}

	void MovementInputController::recordSent(Direction direction, std::int64_t nowMs)
	{
		_lastSent = direction;
		_lastSentAtMs = nowMs;
	}
}
