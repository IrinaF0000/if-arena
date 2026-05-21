#include "ArenaView.hpp"

#include "game/CoordinateTransform.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QtGlobal>

#include <algorithm>
#include <cmath>
#include <utility>

namespace if_arena::battle_qt_client::ui
{
	namespace
	{
		using if_arena::battle_qt_client::game::ArenaSnapshot;
		using if_arena::battle_qt_client::game::ClientIntent;
		using if_arena::battle_qt_client::game::ClientIntentKind;
		using if_arena::battle_qt_client::game::Direction;
		using if_arena::battle_qt_client::game::MapSize;
		using if_arena::battle_qt_client::game::PlayerSnapshot;
		using if_arena::battle_qt_client::game::Team;
		using if_arena::battle_qt_client::game::WorldPoint;
		using if_arena::battle_qt_client::game::clampDirection;
		using if_arena::battle_qt_client::game::teamName;
		using if_arena::battle_qt_client::game::worldToPlayerView;

		QColor playerColor(bool local)
		{
			return local ? QColor{0, 188, 212} : QColor{220, 64, 73};
		}

		QPointF mousePoint(const QMouseEvent* event)
		{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
			return event->position();
#else
			return event->localPos();
#endif
		}
	}

	ArenaView::ArenaView(QWidget* parent)
		: QWidget(parent)
	{
		setMinimumSize(640, 420);
		setMouseTracking(true);
		setFocusPolicy(Qt::StrongFocus);
	}

	void ArenaView::setSnapshot(ArenaSnapshot snapshot, QString localPlayerId)
	{
		_snapshot = std::move(snapshot);
		_localPlayerId = std::move(localPlayerId);
		_lastSnapshotAt.restart();
		update();
	}

	std::optional<PlayerSnapshot> ArenaView::localPlayer() const
	{
		if (!_snapshot.has_value())
		{
			return std::nullopt;
		}
		const auto found = std::find_if(_snapshot->players.begin(), _snapshot->players.end(), [this](const auto& player) {
			return player.playerId == _localPlayerId;
		});
		if (found == _snapshot->players.end())
		{
			return std::nullopt;
		}
		return *found;
	}

	QString ArenaView::hudText() const
	{
		if (!_snapshot.has_value())
		{
			return "Waiting for authoritative snapshot";
		}

		QString blueScore{"0"};
		QString redScore{"0"};
		for (const auto& score : _snapshot->scores)
		{
			if (score.team == Team::Blue)
			{
				blueScore = QString::number(score.score);
			}
			else if (score.team == Team::Red)
			{
				redScore = QString::number(score.score);
			}
		}

		const auto player = localPlayer();
		const auto hp = player.has_value() ? QString::number(player->hp) : "?";
		const auto attack = player.has_value() ? QString::number(player->attackCooldown) : "?";
		const auto dash = player.has_value() ? QString::number(player->dashCooldown) : "?";
		const auto team = player.has_value() ? teamName(player->team) : "unknown";
		const auto age = _lastSnapshotAt.isValid() ? QString::number(_lastSnapshotAt.elapsed()) : "?";
		return "tick " + QString::number(_snapshot->tick) + " | team " + team + " | HP " + hp +
		       " | attack " + attack + " | dash " + dash + " | score B/R " + blueScore + "/" + redScore +
		       " | objective " + _snapshot->objective.state + " | snapshot age " + age + "ms";
	}

	void ArenaView::paintEvent(QPaintEvent*)
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.fillRect(rect(), QColor{18, 24, 32});

		if (!_snapshot.has_value())
		{
			painter.setPen(QColor{220, 225, 230});
			painter.drawText(rect(), Qt::AlignCenter, "No server snapshot yet");
			return;
		}

		drawBoard(painter);
		drawBases(painter);
		drawHazards(painter);
		drawObjective(painter);
		drawPlayers(painter);
	}

	void ArenaView::mouseMoveEvent(QMouseEvent* event)
	{
		emit intentRequested(ClientIntent{ClientIntentKind::Aim, directionFromPoint(mousePoint(event))});
	}

	void ArenaView::mousePressEvent(QMouseEvent* event)
	{
		if (event->button() == Qt::LeftButton)
		{
			emit intentRequested(ClientIntent{ClientIntentKind::Attack, directionFromPoint(mousePoint(event))});
		}
	}

	Team ArenaView::viewerTeam() const
	{
		const auto player = localPlayer();
		return player.has_value() ? player->team : Team::Blue;
	}

	QRectF ArenaView::boardRect() const
	{
		const QRectF available = rect().adjusted(18, 18, -18, -18);
		if (!_snapshot.has_value())
		{
			return available;
		}
		const auto ratio = _snapshot->map.width / _snapshot->map.height;
		double width = available.width();
		double height = width / ratio;
		if (height > available.height())
		{
			height = available.height();
			width = height * ratio;
		}
		return QRectF{available.center().x() - width / 2.0, available.center().y() - height / 2.0, width, height};
	}

	QPointF ArenaView::toScreen(double worldX, double worldY) const
	{
		if (!_snapshot.has_value())
		{
			return {};
		}
		const auto local = worldToPlayerView(WorldPoint{worldX, worldY}, _snapshot->map, viewerTeam());
		const auto board = boardRect();
		const auto cellW = board.width() / _snapshot->map.width;
		const auto cellH = board.height() / _snapshot->map.height;
		return QPointF{board.left() + ((local.x + 0.5) * cellW), board.top() + ((local.y + 0.5) * cellH)};
	}

	Direction ArenaView::directionFromPoint(QPointF point) const
	{
		const auto player = localPlayer();
		if (!player.has_value())
		{
			return Direction{0, -1};
		}
		const auto center = toScreen(player->x, player->y);
		const auto dx = point.x() - center.x();
		const auto dy = point.y() - center.y();
		if (std::abs(dx) < 4.0 && std::abs(dy) < 4.0)
		{
			return Direction{0, -1};
		}
		return clampDirection(dx < -4.0 ? -1 : (dx > 4.0 ? 1 : 0), dy < -4.0 ? -1 : (dy > 4.0 ? 1 : 0));
	}

	void ArenaView::drawBoard(QPainter& painter)
	{
		const auto board = boardRect();
		painter.setPen(QPen{QColor{70, 85, 100}, 1});
		painter.setBrush(QColor{30, 39, 50});
		painter.drawRoundedRect(board, 6, 6);

		const auto cellW = board.width() / _snapshot->map.width;
		const auto cellH = board.height() / _snapshot->map.height;
		painter.setPen(QPen{QColor{50, 62, 76}, 1});
		for (int x = 1; x < static_cast<int>(_snapshot->map.width); ++x)
		{
			const auto px = board.left() + (x * cellW);
			painter.drawLine(QPointF{px, board.top()}, QPointF{px, board.bottom()});
		}
		for (int y = 1; y < static_cast<int>(_snapshot->map.height); ++y)
		{
			const auto py = board.top() + (y * cellH);
			painter.drawLine(QPointF{board.left(), py}, QPointF{board.right(), py});
		}
	}

	void ArenaView::drawBases(QPainter& painter)
	{
		const auto board = boardRect();
		const auto band = board.height() * 0.18;
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor{0, 188, 212, 45});
		painter.drawRect(QRectF{board.left(), board.bottom() - band, board.width(), band});
		painter.setBrush(QColor{220, 64, 73, 45});
		painter.drawRect(QRectF{board.left(), board.top(), board.width(), band});
		painter.setPen(QColor{220, 225, 230});
		painter.drawText(QRectF{board.left(), board.bottom() - band, board.width(), band}, Qt::AlignCenter, "OWN BASE");
		painter.drawText(QRectF{board.left(), board.top(), board.width(), band}, Qt::AlignCenter, "ENEMY BASE");
	}

	void ArenaView::drawObjective(QPainter& painter)
	{
		const auto center = toScreen(_snapshot->objective.x, _snapshot->objective.y);
		painter.setPen(QPen{QColor{255, 255, 255}, 2});
		painter.setBrush(QColor{255, 214, 64});
		painter.drawEllipse(center, 10, 10);
		painter.drawText(QRectF{center.x() - 42, center.y() - 30, 84, 20}, Qt::AlignCenter, _snapshot->objective.state);
	}

	void ArenaView::drawHazards(QPainter& painter)
	{
		for (const auto& hazard : _snapshot->hazards)
		{
			const auto center = toScreen(hazard.x, hazard.y);
			if (hazard.kind == "tower")
			{
				painter.setBrush(hazard.triggered ? QColor{156, 87, 255} : QColor{120, 96, 180});
				painter.setPen(QPen{QColor{230, 220, 255}, 1});
				painter.drawRect(QRectF{center.x() - 8, center.y() - 8, 16, 16});
			}
			else
			{
				painter.setBrush(hazard.triggered ? QColor{255, 145, 77} : QColor{140, 145, 152});
				painter.setPen(QPen{QColor{255, 230, 210}, 1});
				QPolygonF mine;
				mine << QPointF{center.x(), center.y() - 10} << QPointF{center.x() + 10, center.y() + 8}
				     << QPointF{center.x() - 10, center.y() + 8};
				painter.drawPolygon(mine);
			}
		}
	}

	void ArenaView::drawPlayers(QPainter& painter)
	{
		for (const auto& player : _snapshot->players)
		{
			const bool isLocal = player.playerId == _localPlayerId;
			const auto center = toScreen(player.x, player.y);
			painter.setBrush(playerColor(isLocal));
			painter.setPen(QPen{isLocal ? QColor{255, 255, 255} : QColor{45, 20, 24}, isLocal ? 3.0 : 2.0});
			painter.drawEllipse(center, 13, 13);
			painter.setPen(Qt::NoPen);
			painter.setBrush(QColor{30, 30, 30});
			painter.drawRect(QRectF{center.x() - 17, center.y() - 24, 34, 4});
			painter.setBrush(QColor{86, 216, 122});
			painter.drawRect(QRectF{center.x() - 17, center.y() - 24, std::clamp(player.hp, 0, 100) * 0.34, 4});
			painter.setPen(QColor{230, 235, 240});
			painter.drawText(QRectF{center.x() - 34, center.y() + 14, 68, 18}, Qt::AlignCenter,
			                 isLocal ? "YOU" : teamName(player.team).toUpper());
		}
	}
}
