#include "NetworkClient.hpp"

#include "Protocol.hpp"
#include "game/ProtocolJson.hpp"

#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <array>
#include <string>
#include <utility>

namespace if_arena::battle_qt_client::network
{
	namespace
	{
		using if_arena::battle_protocol::ClientSessionPhase;
		using if_arena::battle_protocol::Envelope;
		using if_arena::battle_protocol::MessageType;
		using if_arena::battle_protocol::ProtocolErrorCode;
		using if_arena::battle_protocol::parseEnvelope;
		using if_arena::battle_protocol::serializeEnvelope;
		using if_arena::battle_protocol::validateClientEnvelope;
		using if_arena::battle_qt_client::game::ArenaSnapshot;
		using if_arena::battle_qt_client::game::ClientIntent;
		using if_arena::battle_qt_client::game::ClientIntentKind;

		QByteArray payloadBytes(const QJsonObject& payload)
		{
			return QJsonDocument(payload).toJson(QJsonDocument::Compact);
		}

		QString objectiveActorLabel(const QJsonObject& event, const QString& localSessionId)
		{
			const auto playerId = event.value("playerId").toString();
			return !playerId.isEmpty() && playerId == localSessionId ? QStringLiteral("You") : QStringLiteral("Enemy");
		}

		QString readableEventBatchPayload(const QString& payloadJson, const QString& localSessionId)
		{
			const auto document = QJsonDocument::fromJson(payloadJson.toUtf8());
			if (!document.isObject())
			{
				return "Server events: " + payloadJson;
			}
			const auto events = document.object().value("events");
			if (!events.isArray())
			{
				return "Server events: " + payloadJson;
			}
			for (const auto& value : events.toArray())
			{
				if (!value.isObject())
				{
					continue;
				}
				const auto event = value.toObject();
				const auto type = event.value("type").toString();
				if (type == "objective_picked_up")
				{
					return objectiveActorLabel(event, localSessionId) + " picked up the crystal.";
				}
				if (type == "objective_dropped")
				{
					return objectiveActorLabel(event, localSessionId) + " dropped the crystal.";
				}
				if (type == "objective_captured")
				{
					return objectiveActorLabel(event, localSessionId) + " captured the crystal.";
				}
				if (type == "score_changed")
				{
					return event.value("team").toString("team") + " score " +
					       QString::number(event.value("score").toInt()) + ".";
				}
			}
			return "Server events: " + payloadJson;
		}

		ClientSessionPhase toProtocolPhase(NetworkClient::ProtocolPhase phase)
		{
			switch (phase)
			{
			case NetworkClient::ProtocolPhase::Connected:
				return ClientSessionPhase::Connected;
			case NetworkClient::ProtocolPhase::Authenticated:
				return ClientSessionPhase::Authenticated;
			case NetworkClient::ProtocolPhase::InMatch:
				return ClientSessionPhase::InMatch;
			}
			return ClientSessionPhase::Connected;
		}

		QString errorReasonPayload(const QString& payloadJson)
		{
			QJsonParseError error{};
			const auto document = QJsonDocument::fromJson(payloadJson.toUtf8(), &error);
			if (error.error != QJsonParseError::NoError || !document.isObject())
			{
				return "invalid ack payload";
			}
			const auto object = document.object();
			return object.value("reason").toString("unknown");
		}

		bool acceptedPayload(const QString& payloadJson)
		{
			QJsonParseError error{};
			const auto document = QJsonDocument::fromJson(payloadJson.toUtf8(), &error);
			return error.error == QJsonParseError::NoError && document.isObject() &&
			       document.object().value("accepted").toBool(false);
		}
	}

	NetworkClient::NetworkClient(QObject* parent)
		: QObject(parent)
	{
		connect(&_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
		connect(&_socket, &QTcpSocket::disconnected, this, &NetworkClient::onDisconnected);
		connect(&_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
		connect(&_socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onSocketError);

		_pingTimer.setInterval(3000);
		connect(&_pingTimer, &QTimer::timeout, this, &NetworkClient::sendPing);
	}

	ConnectionState NetworkClient::state() const
	{
		return _state;
	}

	QString NetworkClient::stateText() const
	{
		return connectionStateName(_state);
	}

	QString NetworkClient::sessionId() const
	{
		return _sessionId;
	}

	QString NetworkClient::matchId() const
	{
		return _matchId;
	}

	QString NetworkClient::matchCode() const
	{
		return _matchCode;
	}

	QString NetworkClient::scenarioId() const
	{
		return _scenarioId;
	}

	QString NetworkClient::lastError() const
	{
		return _lastError;
	}

	bool NetworkClient::canSendIntent() const
	{
		return _state == ConnectionState::InMatch && !_matchId.isEmpty();
	}

	void NetworkClient::connectTo(ServerEndpoint endpoint, QString displayName)
	{
		if (_socket.state() != QAbstractSocket::UnconnectedState)
		{
			_socket.abort();
		}

		_endpoint = std::move(endpoint);
		_displayName = displayName.trimmed().isEmpty() ? QString{"qt-player"} : displayName.trimmed().left(64);
		_sessionId.clear();
		_matchId.clear();
		_matchCode.clear();
		_scenarioId.clear();
		_lastError.clear();
		_buffer.clear();
		_sessionSeq = 1;
		_phase = ProtocolPhase::Connected;
		setState(ConnectionState::Connecting);
		_socket.connectToHost(_endpoint.host, _endpoint.port);
	}

	void NetworkClient::disconnectFromServer()
	{
		if (_socket.state() == QAbstractSocket::UnconnectedState)
		{
			setState(ConnectionState::Disconnected);
			return;
		}
		setState(ConnectionState::Disconnecting);
		_pingTimer.stop();
		_socket.disconnectFromHost();
	}

	void NetworkClient::createMatch()
	{
		QJsonObject payload;
		payload.insert("mode", "objective_run");
		payload.insert("scenario", "arena_small_objective_run");
		sendEnvelope(MessageType::CreateMatch, payload);
	}

	void NetworkClient::joinMatch(QString matchCode)
	{
		QJsonObject payload;
		payload.insert("matchCode", matchCode.trimmed().left(32));
		sendEnvelope(MessageType::JoinMatch, payload);
	}

	void NetworkClient::startNextMatch()
	{
		if (_matchId.isEmpty())
		{
			emit eventReceived("Cannot start next match before joining.");
			return;
		}
		QJsonObject payload;
		payload.insert("matchId", _matchId);
		sendEnvelope(MessageType::StartNextMatch, payload);
	}

	void NetworkClient::sendIntent(ClientIntent intent)
	{
		if (intent.kind == ClientIntentKind::Aim)
		{
			return;
		}
		if (!canSendIntent())
		{
			emit eventReceived("Input ignored until match is joined.");
			return;
		}
		if ((intent.kind == ClientIntentKind::Move || intent.kind == ClientIntentKind::Attack ||
		     intent.kind == ClientIntentKind::Dash) &&
		    intent.direction.dx == 0 && intent.direction.dy == 0)
		{
			intent.direction.dy = -1;
		}
		sendEnvelope(MessageType::InputCommand, game::intentPayload(_matchId, intent), _sessionSeq++);
	}

	void NetworkClient::onConnected()
	{
		setState(ConnectionState::Connected);
		_pingTimer.start();
		sendAuthRequest();
	}

	void NetworkClient::onDisconnected()
	{
		_pingTimer.stop();
		_buffer.clear();
		if (_state != ConnectionState::Error)
		{
			setState(ConnectionState::Disconnected);
		}
	}

	void NetworkClient::onReadyRead()
	{
		_buffer.append(_socket.readAll());
		if (_buffer.size() > MaxBufferedBytes)
		{
			fail("Inbound buffer limit exceeded.");
			return;
		}

		while (_buffer.size() >= 4)
		{
			const auto length = (static_cast<quint32>(static_cast<unsigned char>(_buffer[0])) << 24u) |
			                    (static_cast<quint32>(static_cast<unsigned char>(_buffer[1])) << 16u) |
			                    (static_cast<quint32>(static_cast<unsigned char>(_buffer[2])) << 8u) |
			                    static_cast<quint32>(static_cast<unsigned char>(_buffer[3]));
			if (length == 0 || length > MaxFrameBytes)
			{
				fail("Inbound frame length is invalid.");
				return;
			}
			const auto totalLength = static_cast<qsizetype>(4u + length);
			if (_buffer.size() < totalLength)
			{
				return;
			}
			const auto frame = _buffer.mid(4, length);
			_buffer.remove(0, totalLength);
			processFrame(frame);
			if (_state == ConnectionState::Error)
			{
				return;
			}
		}
	}

	void NetworkClient::onSocketError(QAbstractSocket::SocketError)
	{
		if (_state != ConnectionState::Disconnecting && _state != ConnectionState::Disconnected)
		{
			fail("Socket error: " + _socket.errorString());
		}
	}

	void NetworkClient::sendPing()
	{
		if (_socket.state() == QAbstractSocket::ConnectedState)
		{
			_lastPingSentMs = QDateTime::currentMSecsSinceEpoch();
			sendEnvelope(MessageType::Ping, {});
		}
	}

	void NetworkClient::setState(ConnectionState state)
	{
		if (_state == state)
		{
			return;
		}
		_state = state;
		emit connectionStateChanged(_state, connectionStateName(_state));
	}

	void NetworkClient::fail(QString message)
	{
		_lastError = std::move(message);
		setState(ConnectionState::Error);
		emit errorChanged(_lastError);
		_pingTimer.stop();
		_socket.abort();
	}

	void NetworkClient::appendFrame(const QByteArray& payload)
	{
		if (payload.isEmpty() || payload.size() > MaxFrameBytes)
		{
			fail("Outbound frame size is invalid.");
			return;
		}
		std::array<char, 4> header{
			static_cast<char>((payload.size() >> 24) & 0xff),
			static_cast<char>((payload.size() >> 16) & 0xff),
			static_cast<char>((payload.size() >> 8) & 0xff),
			static_cast<char>(payload.size() & 0xff),
		};
		_socket.write(header.data(), static_cast<qint64>(header.size()));
		_socket.write(payload);
	}

	void NetworkClient::processFrame(const QByteArray& frame)
	{
		const std::string json{frame.constData(), static_cast<std::size_t>(frame.size())};
		const auto parsed = parseEnvelope(json);
		if (!parsed.ok())
		{
			fail("Received invalid protocol envelope.");
			return;
		}
		validateServerMessage(parsed.envelope->type, QString::fromStdString(parsed.envelope->payloadJson));
	}

	bool NetworkClient::sendEnvelope(MessageType type, const QJsonObject& payload)
	{
		return sendEnvelope(type, payload, 0);
	}

	bool NetworkClient::sendEnvelope(MessageType type, const QJsonObject& payload, std::uint64_t sessionSeq)
	{
		if (_socket.state() != QAbstractSocket::ConnectedState)
		{
			emit eventReceived("Cannot send while disconnected.");
			return false;
		}

		Envelope envelope;
		envelope.type = type;
		envelope.payloadJson = QString::fromUtf8(payloadBytes(payload)).toStdString();
		if (sessionSeq != 0)
		{
			envelope.sessionSeq = sessionSeq;
		}

		const auto validation = validateClientEnvelope(envelope, toProtocolPhase(_phase));
		if (validation.code != ProtocolErrorCode::None)
		{
			emit errorChanged("Client protocol validation failed: " + QString::fromStdString(validation.message));
			return false;
		}
		const auto serialized = serializeEnvelope(envelope);
		if (!serialized.ok())
		{
			emit errorChanged("Client protocol serialization failed: " + QString::fromStdString(serialized.error.message));
			return false;
		}
		appendFrame(QByteArray::fromStdString(*serialized.json));
		return true;
	}

	void NetworkClient::sendAuthRequest()
	{
		QJsonObject payload;
		payload.insert("mode", "demo");
		payload.insert("displayName", _displayName);
		sendEnvelope(MessageType::AuthRequest, payload);
	}

	void NetworkClient::validateServerMessage(MessageType type, const QString& payloadJson)
	{
		switch (type)
		{
		case MessageType::AuthResult: {
			auto parsed = game::parseSessionIdPayload(payloadJson);
			if (!parsed.ok())
			{
				fail(parsed.error->message);
				return;
			}
			_sessionId = *parsed.value;
			_phase = ProtocolPhase::Authenticated;
			setState(ConnectionState::Authenticated);
			emit eventReceived("Authenticated as player " + _sessionId + ".");
			return;
		}
		case MessageType::MatchJoined: {
			auto parsed = game::parseMatchJoinedPayload(payloadJson);
			if (!parsed.ok())
			{
				fail(parsed.error->message);
				return;
			}
			_matchId = parsed.value->matchId;
			_matchCode = parsed.value->matchCode;
			_scenarioId = parsed.value->scenario.id;
			_sessionSeq = 1;
			_phase = ProtocolPhase::InMatch;
			setState(ConnectionState::InMatch);
			emit matchJoined(_matchId, _matchCode, _scenarioId);
			emit eventReceived("Joined match " + _matchId + " with code " + _matchCode + " scenario " + _scenarioId + ".");
			return;
		}
		case MessageType::Snapshot: {
			auto parsed = game::parseArenaSnapshotPayload(payloadJson);
			if (!parsed.ok())
			{
				fail(parsed.error->message);
				return;
			}
			emit snapshotReceived(*parsed.value, _sessionId);
			return;
		}
		case MessageType::EventBatch:
			emit eventReceived(readableEventBatchPayload(payloadJson, _sessionId));
			return;
		case MessageType::InputAck: {
			const bool accepted = acceptedPayload(payloadJson);
			const auto reason = errorReasonPayload(payloadJson);
			emit inputAckReceived(accepted, reason);
			if (!accepted)
			{
				emit eventReceived("Input rejected: " + reason + ".");
			}
			return;
		}
		case MessageType::Ping:
			sendEnvelope(MessageType::Pong, {});
			return;
		case MessageType::Pong:
			if (_lastPingSentMs != 0)
			{
				emit latencyUpdated(static_cast<int>(QDateTime::currentMSecsSinceEpoch() - _lastPingSentMs));
			}
			return;
		case MessageType::Error: {
			auto parsed = game::parseErrorMessagePayload(payloadJson);
			fail(parsed.ok() ? *parsed.value : QString{"Server returned an invalid error payload."});
			return;
		}
		case MessageType::AuthRequest:
		case MessageType::CreateMatch:
		case MessageType::JoinMatch:
		case MessageType::StartNextMatch:
		case MessageType::InputCommand:
		case MessageType::Handshake:
			fail("Server sent a client-originated message type.");
			return;
		}
		fail("Unknown server message.");
	}

	QString connectionStateName(ConnectionState state)
	{
		switch (state)
		{
		case ConnectionState::Disconnected:
			return "disconnected";
		case ConnectionState::Connecting:
			return "connecting";
		case ConnectionState::Connected:
			return "connected";
		case ConnectionState::Authenticated:
			return "authenticated";
		case ConnectionState::InMatch:
			return "in match";
		case ConnectionState::Disconnecting:
			return "disconnecting";
		case ConnectionState::Error:
			return "error";
		}
		return "unknown";
	}
}
