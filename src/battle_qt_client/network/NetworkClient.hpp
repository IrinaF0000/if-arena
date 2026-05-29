#pragma once

#include "Protocol.hpp"
#include "game/ClientTypes.hpp"

#include <QJsonObject>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>

#include <cstdint>

namespace if_arena::battle_qt_client::network
{
	enum class ConnectionState
	{
		Disconnected,
		Connecting,
		Connected,
		Authenticated,
		InMatch,
		Disconnecting,
		Error
	};

	struct ServerEndpoint
	{
		QString host{"127.0.0.1"};
		quint16 port{5555};
	};

	class NetworkClient : public QObject
	{
		Q_OBJECT

	public:
		enum class ProtocolPhase
		{
			Connected,
			Authenticated,
			InMatch
		};

		explicit NetworkClient(QObject* parent = nullptr);

		[[nodiscard]] ConnectionState state() const;
		[[nodiscard]] QString stateText() const;
		[[nodiscard]] QString sessionId() const;
		[[nodiscard]] QString matchId() const;
		[[nodiscard]] QString matchCode() const;
		[[nodiscard]] QString scenarioId() const;
		[[nodiscard]] QString lastError() const;
		[[nodiscard]] bool canSendIntent() const;

	public slots:
		void connectTo(ServerEndpoint endpoint, QString displayName);
		void disconnectFromServer();
		void createMatch();
		void joinMatch(QString matchCode);
		void sendIntent(if_arena::battle_qt_client::game::ClientIntent intent);

	signals:
		void connectionStateChanged(if_arena::battle_qt_client::network::ConnectionState state, QString label);
		void errorChanged(QString message);
		void eventReceived(QString message);
		void matchJoined(QString matchId, QString matchCode, QString scenarioId);
		void snapshotReceived(if_arena::battle_qt_client::game::ArenaSnapshot snapshot, QString localPlayerId);
		void inputAckReceived(bool accepted, QString reason);
		void latencyUpdated(int milliseconds);

	private slots:
		void onConnected();
		void onDisconnected();
		void onReadyRead();
		void onSocketError(QAbstractSocket::SocketError error);
		void sendPing();

	private:
		void setState(ConnectionState state);
		void fail(QString message);
		void appendFrame(const QByteArray& payload);
		void processFrame(const QByteArray& frame);
		bool sendEnvelope(if_arena::battle_protocol::MessageType type, const QJsonObject& payload);
		bool sendEnvelope(if_arena::battle_protocol::MessageType type, const QJsonObject& payload, std::uint64_t sessionSeq);
		void sendAuthRequest();
		void validateServerMessage(if_arena::battle_protocol::MessageType type, const QString& payloadJson);

		QTcpSocket _socket;
		QTimer _pingTimer;
		QByteArray _buffer;
		ConnectionState _state{ConnectionState::Disconnected};
		ProtocolPhase _phase{ProtocolPhase::Connected};
		ServerEndpoint _endpoint;
		QString _displayName{"qt-player"};
		QString _sessionId;
		QString _matchId;
		QString _matchCode;
		QString _scenarioId;
		QString _lastError;
		std::uint64_t _sessionSeq{1};
		qint64 _lastPingSentMs{};
		static constexpr qsizetype MaxFrameBytes = 64 * 1024;
		static constexpr qsizetype MaxBufferedBytes = 128 * 1024;
	};

	[[nodiscard]] QString connectionStateName(ConnectionState state);
}
