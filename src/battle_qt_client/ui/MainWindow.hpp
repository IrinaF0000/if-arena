#pragma once

#include "ArenaView.hpp"
#include "game/MovementInputController.hpp"
#include "network/NetworkClient.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

#include <set>

namespace if_arena::battle_qt_client::ui
{
	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit MainWindow(QWidget* parent = nullptr);

	protected:
		bool eventFilter(QObject* watched, QEvent* event) override;
		void keyPressEvent(QKeyEvent* event) override;
		void keyReleaseEvent(QKeyEvent* event) override;

	private:
		void wireSignals();
		void refreshControls();
		void appendEvent(const QString& message);
		void sendMovement();
		void pollHeldMovement();
		void sendMovementCommand(if_arena::battle_qt_client::game::MovementInputCommand command);
		void sendAction(if_arena::battle_qt_client::game::ClientIntentKind kind);
		[[nodiscard]] bool handleGameplayKeyPress(QKeyEvent* event);
		[[nodiscard]] bool handleGameplayKeyRelease(QKeyEvent* event);
		[[nodiscard]] if_arena::battle_qt_client::game::Direction currentMoveDirection() const;
		[[nodiscard]] if_arena::battle_qt_client::game::Direction currentActionDirection() const;

		network::NetworkClient _client;
		ArenaView* _arena{};
		QLineEdit* _host{};
		QSpinBox* _port{};
		QLineEdit* _displayName{};
		QLineEdit* _joinCode{};
		QPushButton* _connect{};
		QPushButton* _disconnect{};
		QPushButton* _create{};
		QPushButton* _join{};
		QLabel* _connection{};
		QLabel* _identity{};
		QLabel* _hud{};
		QLabel* _controlsHint{};
		QLabel* _error{};
		QPlainTextEdit* _events{};
		std::set<int> _movementKeys;
		QTimer _movementResendTimer;
		if_arena::battle_qt_client::game::MovementInputController _movementInput;
		if_arena::battle_qt_client::game::Direction _aimDirection{0, -1};
		bool _hasAimDirection{};
	};
}
