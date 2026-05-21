#include "MainWindow.hpp"

#include "game/CoordinateTransform.hpp"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

namespace if_arena::battle_qt_client::ui
{
	namespace
	{
		using if_arena::battle_qt_client::game::ClientIntent;
		using if_arena::battle_qt_client::game::ClientIntentKind;
		using if_arena::battle_qt_client::game::Direction;
		using if_arena::battle_qt_client::game::clampDirection;

		bool isMovementKey(int key)
		{
			return key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D ||
			       key == Qt::Key_Up || key == Qt::Key_Left || key == Qt::Key_Down || key == Qt::Key_Right;
		}

		bool sameDirection(Direction lhs, Direction rhs)
		{
			return lhs.dx == rhs.dx && lhs.dy == rhs.dy;
		}
	}

	MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
	{
		setWindowTitle("IF Arena Qt Client");
		setMinimumSize(980, 720);

		auto* root = new QWidget(this);
		auto* layout = new QVBoxLayout(root);

		auto* connectionRow = new QHBoxLayout();
		_host = new QLineEdit("127.0.0.1", root);
		_port = new QSpinBox(root);
		_port->setRange(1, 65535);
		_port->setValue(5555);
		_displayName = new QLineEdit("qt-player", root);
		_connect = new QPushButton("Connect", root);
		_disconnect = new QPushButton("Disconnect", root);
		connectionRow->addWidget(new QLabel("Host", root));
		connectionRow->addWidget(_host, 2);
		connectionRow->addWidget(new QLabel("Port", root));
		connectionRow->addWidget(_port);
		connectionRow->addWidget(new QLabel("Name", root));
		connectionRow->addWidget(_displayName, 2);
		connectionRow->addWidget(_connect);
		connectionRow->addWidget(_disconnect);

		auto* lobbyRow = new QHBoxLayout();
		_create = new QPushButton("Create", root);
		_joinCode = new QLineEdit(root);
		_joinCode->setPlaceholderText("Match code");
		_join = new QPushButton("Join", root);
		lobbyRow->addWidget(_create);
		lobbyRow->addWidget(_joinCode, 1);
		lobbyRow->addWidget(_join);

		auto* statusRow = new QHBoxLayout();
		_connection = new QLabel("disconnected", root);
		_identity = new QLabel("session -, match -", root);
		_hud = new QLabel("Waiting for authoritative snapshot", root);
		_controlsHint = new QLabel("WASD/arrows move | mouse aims | left click/Space attack | right click/Shift dash", root);
		_error = new QLabel(root);
		_controlsHint->setStyleSheet("color: #aeb8c4;");
		_error->setStyleSheet("color: #ff8a80;");
		statusRow->addWidget(_connection);
		statusRow->addWidget(_identity);
		statusRow->addWidget(_hud, 1);

		_arena = new ArenaView(root);
		_events = new QPlainTextEdit(root);
		_events->setReadOnly(true);
		_events->setMaximumBlockCount(120);
		_events->setMaximumHeight(140);

		layout->addLayout(connectionRow);
		layout->addLayout(lobbyRow);
		layout->addLayout(statusRow);
		layout->addWidget(_controlsHint);
		layout->addWidget(_error);
		layout->addWidget(_arena, 1);
		layout->addWidget(_events);
		setCentralWidget(root);
		setFocusPolicy(Qt::StrongFocus);

		wireSignals();
		refreshControls();
	}

	void MainWindow::keyPressEvent(QKeyEvent* event)
	{
		if (event->isAutoRepeat())
		{
			return;
		}
		if (isMovementKey(event->key()))
		{
			_movementKeys.insert(event->key());
			sendMovement();
			event->accept();
			return;
		}
		if (event->key() == Qt::Key_Space)
		{
			sendAction(ClientIntentKind::Attack);
			event->accept();
			return;
		}
		if (event->key() == Qt::Key_Shift)
		{
			sendAction(ClientIntentKind::Dash);
			event->accept();
			return;
		}
		if (event->key() == Qt::Key_Escape)
		{
			_client.disconnectFromServer();
			event->accept();
			return;
		}
		QMainWindow::keyPressEvent(event);
	}

	void MainWindow::keyReleaseEvent(QKeyEvent* event)
	{
		if (event->isAutoRepeat())
		{
			return;
		}
		if (isMovementKey(event->key()))
		{
			_movementKeys.erase(event->key());
			sendMovement();
			event->accept();
			return;
		}
		QMainWindow::keyReleaseEvent(event);
	}

	void MainWindow::wireSignals()
	{
		connect(_connect, &QPushButton::clicked, this, [this] {
			_client.connectTo(network::ServerEndpoint{_host->text().trimmed(), static_cast<quint16>(_port->value())},
			                  _displayName->text());
		});
		connect(_disconnect, &QPushButton::clicked, &_client, &network::NetworkClient::disconnectFromServer);
		connect(_create, &QPushButton::clicked, &_client, &network::NetworkClient::createMatch);
		connect(_join, &QPushButton::clicked, this, [this] { _client.joinMatch(_joinCode->text()); });
		connect(_arena, &ArenaView::aimChanged, this, [this](Direction direction) {
			_aimDirection = direction;
			_hasAimDirection = true;
		});
		connect(_arena, &ArenaView::intentRequested, this, [this](ClientIntent intent) {
			if (intent.direction.dx == 0 && intent.direction.dy == 0)
			{
				intent.direction = currentActionDirection();
			}
			if (intent.kind == ClientIntentKind::Attack)
			{
				_arena->showAttackFeedback(intent.direction);
			}
			if (intent.kind == ClientIntentKind::Dash)
			{
				_arena->showDashFeedback(intent.direction);
			}
			_client.sendIntent(intent);
		});

		connect(&_client, &network::NetworkClient::connectionStateChanged, this,
		        [this](network::ConnectionState, const QString& label) {
			        _connection->setText("connection " + label);
			        _identity->setText("session " + _client.sessionId() + ", match " + _client.matchId());
			        refreshControls();
		        });
		connect(&_client, &network::NetworkClient::errorChanged, this, [this](const QString& message) {
			_error->setText(message);
			appendEvent("error: " + message);
			refreshControls();
		});
		connect(&_client, &network::NetworkClient::eventReceived, this, &MainWindow::appendEvent);
		connect(&_client, &network::NetworkClient::matchJoined, this, [this](const QString& matchId, const QString& code) {
			_joinCode->setText(code);
			_identity->setText("session " + _client.sessionId() + ", match " + matchId + ", code " + code);
			_movementKeys.clear();
			_lastMovementIntent.reset();
			_hasAimDirection = false;
			_arena->setFocus(Qt::OtherFocusReason);
			refreshControls();
		});
		connect(&_client, &network::NetworkClient::snapshotReceived, this,
		        [this](const game::ArenaSnapshot& snapshot, const QString& localPlayerId) {
			        _arena->setSnapshot(snapshot, localPlayerId);
			        _hud->setText(_arena->hudText());
		        });
		connect(&_client, &network::NetworkClient::inputAckReceived, this,
		        [this](bool accepted, const QString& reason) {
			        if (!accepted)
			        {
				        appendEvent("input rejected (" + reason + ")");
			        }
		        });
		connect(&_client, &network::NetworkClient::latencyUpdated, this,
		        [this](int milliseconds) { appendEvent("ping " + QString::number(milliseconds) + "ms"); });
	}

	void MainWindow::refreshControls()
	{
		const auto state = _client.state();
		const bool disconnected = state == network::ConnectionState::Disconnected || state == network::ConnectionState::Error;
		const bool authenticated = state == network::ConnectionState::Authenticated;
		const bool inMatch = state == network::ConnectionState::InMatch;
		_connect->setEnabled(disconnected);
		_disconnect->setEnabled(!disconnected);
		_create->setEnabled(authenticated);
		_join->setEnabled(authenticated);
		_joinCode->setEnabled(authenticated || inMatch);
		_error->setVisible(!_error->text().isEmpty());
	}

	void MainWindow::appendEvent(const QString& message)
	{
		_events->appendPlainText(message);
	}

	void MainWindow::sendMovement()
	{
		const auto direction = currentMoveDirection();
		if (direction.dx == 0 && direction.dy == 0)
		{
			if (!_lastMovementIntent.has_value())
			{
				return;
			}
			_lastMovementIntent.reset();
			_client.sendIntent(ClientIntent{ClientIntentKind::Stop, {}});
			return;
		}
		if (_lastMovementIntent.has_value() && sameDirection(*_lastMovementIntent, direction))
		{
			return;
		}
		_lastMovementIntent = direction;
		_client.sendIntent(ClientIntent{ClientIntentKind::Move, direction});
	}

	void MainWindow::sendAction(ClientIntentKind kind)
	{
		const auto direction = currentActionDirection();
		if (kind == ClientIntentKind::Attack)
		{
			_arena->showAttackFeedback(direction);
		}
		if (kind == ClientIntentKind::Dash)
		{
			_arena->showDashFeedback(direction);
		}
		_client.sendIntent(ClientIntent{kind, direction});
	}

	Direction MainWindow::currentMoveDirection() const
	{
		int dx = 0;
		int dy = 0;
		if (_movementKeys.contains(Qt::Key_W) || _movementKeys.contains(Qt::Key_Up))
		{
			--dy;
		}
		if (_movementKeys.contains(Qt::Key_S) || _movementKeys.contains(Qt::Key_Down))
		{
			++dy;
		}
		if (_movementKeys.contains(Qt::Key_A) || _movementKeys.contains(Qt::Key_Left))
		{
			--dx;
		}
		if (_movementKeys.contains(Qt::Key_D) || _movementKeys.contains(Qt::Key_Right))
		{
			++dx;
		}
		return clampDirection(dx, dy);
	}

	Direction MainWindow::currentActionDirection() const
	{
		if (_hasAimDirection && (_aimDirection.dx != 0 || _aimDirection.dy != 0))
		{
			return _aimDirection;
		}
		const auto movement = currentMoveDirection();
		if (movement.dx != 0 || movement.dy != 0)
		{
			return movement;
		}
		return Direction{0, -1};
	}
}
