#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace if_arena::battle_qt_client::network
{
	struct ServerEndpoint
	{
		std::string host{"127.0.0.1"};
		std::uint16_t port{4000};
	};

	enum class ConnectionState
	{
		Disconnected,
		Connecting,
		Connected,
		Disconnecting,
		Error
	};

	enum class ClientIntentKind
	{
		Move,
		Aim,
		Attack,
		Dash,
		Interact,
		Stop
	};

	struct ClientIntent
	{
		ClientIntentKind kind{ClientIntentKind::Stop};
		float x{};
		float y{};
	};

	class NetworkClient
	{
	public:
		[[nodiscard]] ConnectionState state() const
		{
			return _state;
		}

		[[nodiscard]] const ServerEndpoint& endpoint() const
		{
			return _endpoint;
		}

		[[nodiscard]] std::string_view lastError() const
		{
			return _lastError;
		}

		[[nodiscard]] bool canSendIntent() const
		{
			return _state == ConnectionState::Connected;
		}

		void connectTo(ServerEndpoint endpoint)
		{
			_endpoint = std::move(endpoint);
			_lastError.clear();
			_state = ConnectionState::Connecting;
		}

		void markConnectedForAdapter()
		{
			_lastError.clear();
			_state = ConnectionState::Connected;
		}

		void failConnection(std::string error)
		{
			_lastError = std::move(error);
			_state = ConnectionState::Error;
		}

		void disconnect()
		{
			_state = ConnectionState::Disconnected;
		}

		[[nodiscard]] bool sendIntent(ClientIntent)
		{
			return canSendIntent();
		}

	private:
		ConnectionState _state{ConnectionState::Disconnected};
		ServerEndpoint _endpoint;
		std::string _lastError;
	};
}
