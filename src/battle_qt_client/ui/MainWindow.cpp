#include "MainWindow.hpp"

#include "game/CoordinateTransform.hpp"

#include <QDateTime>
#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QVBoxLayout>

#include <array>

namespace if_arena::battle_qt_client::ui
{
	namespace
	{
		using if_arena::battle_qt_client::game::ClientIntent;
		using if_arena::battle_qt_client::game::ClientIntentKind;
		using if_arena::battle_qt_client::game::Direction;
		using if_arena::battle_qt_client::game::MovementInputCommand;
		using if_arena::battle_qt_client::game::MovementInputCommandKind;
		using if_arena::battle_qt_client::game::clampDirection;

		bool isMovementKey(int key)
		{
			return key == Qt::Key_W || key == Qt::Key_A || key == Qt::Key_S || key == Qt::Key_D ||
			       key == Qt::Key_Up || key == Qt::Key_Left || key == Qt::Key_Down || key == Qt::Key_Right;
		}

		std::int64_t nowMs()
		{
			return QDateTime::currentMSecsSinceEpoch();
		}
	}

	MainWindow::MainWindow(QWidget* parent)
		: QMainWindow(parent)
	{
		setWindowTitle("IF Arena Qt Client");
		setMinimumSize(1180, 720);

		auto* root = new QWidget(this);
		auto* layout = new QHBoxLayout(root);
		layout->setContentsMargins(10, 10, 10, 10);
		layout->setSpacing(10);

		_arena = new ArenaView(root);
		layout->addWidget(_arena, 1);

		auto* sidePanel = new QWidget(root);
		sidePanel->setMinimumWidth(320);
		sidePanel->setMaximumWidth(380);
		auto* sideLayout = new QVBoxLayout(sidePanel);
		sideLayout->setContentsMargins(0, 0, 0, 0);
		sideLayout->setSpacing(8);

		auto* connectionBox = new QGroupBox("Connection", sidePanel);
		auto* connectionForm = new QFormLayout(connectionBox);
		_host = new QLineEdit("127.0.0.1", root);
		_port = new QSpinBox(root);
		_port->setRange(1, 65535);
		_port->setValue(5555);
		_displayName = new QLineEdit("qt-player", root);
		_connect = new QPushButton("Connect", root);
		_disconnect = new QPushButton("Disconnect", root);
		auto* connectionButtons = new QHBoxLayout();
		connectionButtons->addWidget(_connect);
		connectionButtons->addWidget(_disconnect);
		connectionForm->addRow("Host", _host);
		connectionForm->addRow("Port", _port);
		connectionForm->addRow("Name", _displayName);
		connectionForm->addRow(connectionButtons);

		auto* lobbyBox = new QGroupBox("Match", sidePanel);
		auto* lobbyLayout = new QVBoxLayout(lobbyBox);
		_create = new QPushButton("Create", root);
		_joinCode = new QLineEdit(root);
		_joinCode->setPlaceholderText("Match code");
		_join = new QPushButton("Join", root);
		_nextMatch = new QPushButton("Next match", root);
		auto* joinRow = new QHBoxLayout();
		joinRow->addWidget(_joinCode, 1);
		joinRow->addWidget(_join);
		lobbyLayout->addWidget(_create);
		lobbyLayout->addLayout(joinRow);
		lobbyLayout->addWidget(_nextMatch);

		auto* statusBox = new QGroupBox("Status", sidePanel);
		auto* statusLayout = new QVBoxLayout(statusBox);
		_connection = new QLabel("disconnected", root);
		_identity = new QLabel("session -, match -", root);
		_hud = new QLabel("Waiting for authoritative snapshot", root);
		_controlsHint = new QLabel("WASD/arrows move | mouse aims | left click/Space attack | right click/Shift dash", root);
		_error = new QLabel(root);
		_connection->setWordWrap(true);
		_identity->setWordWrap(true);
		_hud->setWordWrap(true);
		_controlsHint->setWordWrap(true);
		_error->setWordWrap(true);
		_controlsHint->setStyleSheet("color: #aeb8c4;");
		_error->setStyleSheet("color: #ff8a80;");
		statusLayout->addWidget(_connection);
		statusLayout->addWidget(_identity);
		statusLayout->addWidget(_hud);
		statusLayout->addWidget(_controlsHint);
		statusLayout->addWidget(_error);

		auto* logBox = new QGroupBox("Event log", sidePanel);
		auto* logLayout = new QVBoxLayout(logBox);
		_events = new QPlainTextEdit(root);
		_events->setReadOnly(true);
		_events->setMaximumBlockCount(120);
		_events->setMinimumHeight(160);
		logLayout->addWidget(_events);

		sideLayout->addWidget(connectionBox);
		sideLayout->addWidget(lobbyBox);
		sideLayout->addWidget(statusBox);
		sideLayout->addWidget(logBox, 1);
		layout->addWidget(sidePanel);
		setCentralWidget(root);
		setFocusPolicy(Qt::StrongFocus);
		for (auto* button : {_connect, _disconnect, _create, _join, _nextMatch})
		{
			button->setFocusPolicy(Qt::NoFocus);
			button->installEventFilter(this);
		}
		const std::array<QWidget*, 7> gameplayKeyWidgets{root, _host, _port, _displayName, _joinCode, _arena, _events};
		for (auto* widget : gameplayKeyWidgets)
		{
			widget->installEventFilter(this);
		}

		wireSignals();
		_movementResendTimer.setInterval(static_cast<int>(_movementInput.resendIntervalMs()));
		connect(&_movementResendTimer, &QTimer::timeout, this, &MainWindow::pollHeldMovement);
		refreshControls();
	}

	bool MainWindow::eventFilter(QObject* watched, QEvent* event)
	{
		if (_client.state() == network::ConnectionState::InMatch)
		{
			if (event->type() == QEvent::KeyPress && handleGameplayKeyPress(static_cast<QKeyEvent*>(event)))
			{
				return true;
			}
			if (event->type() == QEvent::KeyRelease && handleGameplayKeyRelease(static_cast<QKeyEvent*>(event)))
			{
				return true;
			}
		}
		return QMainWindow::eventFilter(watched, event);
	}

	void MainWindow::keyPressEvent(QKeyEvent* event)
	{
		if (handleGameplayKeyPress(event))
		{
			return;
		}
		QMainWindow::keyPressEvent(event);
	}

	void MainWindow::keyReleaseEvent(QKeyEvent* event)
	{
		if (handleGameplayKeyRelease(event))
		{
			return;
		}
		QMainWindow::keyReleaseEvent(event);
	}

	bool MainWindow::handleGameplayKeyPress(QKeyEvent* event)
	{
		if (event->isAutoRepeat())
		{
			return false;
		}
		if (isMovementKey(event->key()))
		{
			_movementKeys.insert(event->key());
			sendMovement();
			event->accept();
			return true;
		}
		if (event->key() == Qt::Key_Space)
		{
			sendAction(ClientIntentKind::Attack);
			event->accept();
			return true;
		}
		if (event->key() == Qt::Key_Shift)
		{
			sendAction(ClientIntentKind::Dash);
			event->accept();
			return true;
		}
		if (event->key() == Qt::Key_Escape)
		{
			_client.disconnectFromServer();
			event->accept();
			return true;
		}
		return false;
	}

	bool MainWindow::handleGameplayKeyRelease(QKeyEvent* event)
	{
		if (event->isAutoRepeat())
		{
			return false;
		}
		if (isMovementKey(event->key()))
		{
			_movementKeys.erase(event->key());
			sendMovement();
			event->accept();
			return true;
		}
		return false;
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
		connect(_nextMatch, &QPushButton::clicked, &_client, &network::NetworkClient::startNextMatch);
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
		        [this](network::ConnectionState state, const QString& label) {
			        _connection->setText("connection " + label);
			        _identity->setText("session " + _client.sessionId() + ", match " + _client.matchId() +
			                           (_client.scenarioId().isEmpty() ? QString{} : ", scenario " + _client.scenarioId()));
			        if (state != network::ConnectionState::InMatch)
			        {
				        _matchFinished = false;
			        }
			        refreshControls();
		        });
		connect(&_client, &network::NetworkClient::errorChanged, this, [this](const QString& message) {
			_error->setText(message);
			appendEvent("error: " + message);
			refreshControls();
		});
		connect(&_client, &network::NetworkClient::eventReceived, this, &MainWindow::appendEvent);
		connect(&_client, &network::NetworkClient::matchJoined, this,
		        [this](const QString& matchId, const QString& code, const QString& scenarioId) {
			_joinCode->setText(code);
			_identity->setText("session " + _client.sessionId() + ", match " + matchId + ", code " + code +
			                   ", scenario " + scenarioId);
			_movementKeys.clear();
			_movementInput.reset();
			_hasAimDirection = false;
			_matchFinished = false;
			_arena->setFocus(Qt::OtherFocusReason);
			refreshControls();
		});
		connect(&_client, &network::NetworkClient::snapshotReceived, this,
		        [this](const game::ArenaSnapshot& snapshot, const QString& localPlayerId) {
			        _arena->setSnapshot(snapshot, localPlayerId);
			        _hud->setText(_arena->hudText());
			        _matchFinished = snapshot.finished;
			        refreshControls();
		        });
		connect(&_client, &network::NetworkClient::inputAckReceived, this,
		        [this](bool accepted, const QString& reason) {
			        if (!accepted)
			        {
				        _movementInput.markRejected();
				        appendEvent("input rejected (" + reason + ")");
				        _error->setText("movement blocked: " + reason);
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
		_nextMatch->setEnabled(inMatch && _matchFinished);
		_joinCode->setEnabled(authenticated || inMatch);
		if (inMatch && !_movementResendTimer.isActive())
		{
			_movementResendTimer.start();
		}
		if (!inMatch && _movementResendTimer.isActive())
		{
			_movementResendTimer.stop();
			_movementKeys.clear();
			_movementInput.reset();
		}
		_error->setVisible(!_error->text().isEmpty());
	}

	void MainWindow::appendEvent(const QString& message)
	{
		_events->appendPlainText(message);
	}

	void MainWindow::sendMovement()
	{
		const auto direction = currentMoveDirection();
		const auto command = _movementInput.updateDesired(direction, nowMs());
		if (command.has_value())
		{
			sendMovementCommand(*command);
		}
	}

	void MainWindow::pollHeldMovement()
	{
		const auto command = _movementInput.poll(nowMs());
		if (command.has_value())
		{
			sendMovementCommand(*command);
		}
	}

	void MainWindow::sendMovementCommand(MovementInputCommand command)
	{
		const ClientIntentKind kind =
		    command.kind == MovementInputCommandKind::Move ? ClientIntentKind::Move : ClientIntentKind::Stop;
		_client.sendIntent(ClientIntent{kind, command.direction});
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
