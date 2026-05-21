#pragma once

#include "game/ClientTypes.hpp"

#include <QElapsedTimer>
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
		[[nodiscard]] std::optional<if_arena::battle_qt_client::game::PlayerSnapshot> localPlayer() const;
		[[nodiscard]] QString hudText() const;

	signals:
		void intentRequested(if_arena::battle_qt_client::game::ClientIntent intent);

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
		void drawPlayers(QPainter& painter);

		std::optional<if_arena::battle_qt_client::game::ArenaSnapshot> _snapshot;
		QString _localPlayerId;
		QElapsedTimer _lastSnapshotAt;
	};
}
