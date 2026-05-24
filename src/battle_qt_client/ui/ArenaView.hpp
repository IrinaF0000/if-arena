#pragma once

#include "game/ClientTypes.hpp"

#include <QElapsedTimer>
#include <QSvgRenderer>
#include <QWidget>

#include <optional>

namespace if_arena::battle_qt_client::ui
{
	class ArenaView : public QWidget
	{
		Q_OBJECT

	public:
		explicit ArenaView(QWidget* parent = nullptr);

		void setSnapshot(if_arena::battle_qt_client::game::ArenaSnapshot snapshot, QString localPlayerId);
		void showAttackFeedback(if_arena::battle_qt_client::game::Direction direction);
		void showDashFeedback(if_arena::battle_qt_client::game::Direction direction);
		[[nodiscard]] std::optional<if_arena::battle_qt_client::game::PlayerSnapshot> localPlayer() const;
		[[nodiscard]] QString hudText() const;

	signals:
		void intentRequested(if_arena::battle_qt_client::game::ClientIntent intent);
		void aimChanged(if_arena::battle_qt_client::game::Direction direction);

	protected:
		void paintEvent(QPaintEvent* event) override;
		void mouseMoveEvent(QMouseEvent* event) override;
		void mousePressEvent(QMouseEvent* event) override;

	private:
		[[nodiscard]] if_arena::battle_qt_client::game::Team viewerTeam() const;
		[[nodiscard]] QRectF boardRect() const;
		[[nodiscard]] QPointF toScreen(double worldX, double worldY) const;
		[[nodiscard]] if_arena::battle_qt_client::game::Direction directionFromPoint(QPointF point) const;
		void drawBoard(QPainter& painter);
		void drawBases(QPainter& painter);
		void drawObjective(QPainter& painter);
		void drawHazards(QPainter& painter);
		void drawLocalActionPreview(QPainter& painter);
		void drawPlayers(QPainter& painter);
		void drawPlayerSprite(QPainter& painter, QPointF center, double size,
		                      if_arena::battle_qt_client::game::Direction facing, bool isLocal);
		[[nodiscard]] QPointF directionEnd(QPointF origin, if_arena::battle_qt_client::game::Direction direction,
		                                    double length) const;
		[[nodiscard]] double directionAngle(if_arena::battle_qt_client::game::Direction direction) const;

		std::optional<if_arena::battle_qt_client::game::ArenaSnapshot> _snapshot;
		QString _localPlayerId;
		QSvgRenderer _playerSprite;
		QElapsedTimer _lastSnapshotAt;
		QElapsedTimer _lastAttackFeedbackAt;
		QElapsedTimer _lastDashFeedbackAt;
		if_arena::battle_qt_client::game::Direction _aimDirection{0, -1};
		if_arena::battle_qt_client::game::Direction _attackFeedbackDirection{0, -1};
		if_arena::battle_qt_client::game::Direction _dashFeedbackDirection{0, -1};
	};
}
