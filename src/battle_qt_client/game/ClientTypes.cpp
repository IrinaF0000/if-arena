#include "ClientTypes.hpp"

namespace if_arena::battle_qt_client::game
{
	Team teamFromString(const QString& value)
	{
		if (value == "blue")
		{
			return Team::Blue;
		}
		if (value == "red")
		{
			return Team::Red;
		}
		return Team::Unknown;
	}

	QString teamName(Team team)
	{
		switch (team)
		{
		case Team::Blue:
			return "blue";
		case Team::Red:
			return "red";
		case Team::Unknown:
			return "unknown";
		}
		return "unknown";
	}

	QString intentKindName(ClientIntentKind kind)
	{
		switch (kind)
		{
		case ClientIntentKind::Move:
			return "move";
		case ClientIntentKind::Aim:
			return "aim";
		case ClientIntentKind::Attack:
			return "attack";
		case ClientIntentKind::Dash:
			return "dash";
		case ClientIntentKind::Interact:
			return "interact";
		case ClientIntentKind::Stop:
			return "stop";
		}
		return "stop";
	}
}
