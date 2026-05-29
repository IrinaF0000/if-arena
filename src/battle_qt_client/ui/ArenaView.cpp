#include "ArenaView.hpp"

#include "game/CoordinateTransform.hpp"

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStringList>
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
			return local ? QColor{42, 190, 205} : QColor{226, 82, 88};
		}

		QColor teamColor(Team team)
		{
			if (team == Team::Blue)
			{
				return QColor{42, 190, 205};
			}
			if (team == Team::Red)
			{
				return QColor{226, 82, 88};
			}
			return QColor{180, 190, 200};
		}

		QString hazardIconKind(const game::HazardSnapshot& hazard)
		{
			if (hazard.icon.contains("tower"))
			{
				return QStringLiteral("tower");
			}
			if (hazard.icon.contains("crow") || hazard.icon.contains("drone"))
			{
				return QStringLiteral("crow");
			}
			return QStringLiteral("mine");
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
		: QWidget(parent),
		  _playerSprite(QStringLiteral(":/if_arena/assets/players/swordsman.svg"))
	{
		setMinimumSize(820, 520);
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

	void ArenaView::showAttackFeedback(Direction direction)
	{
		_attackFeedbackDirection = direction;
		_lastAttackFeedbackAt.restart();
		update();
	}

	void ArenaView::showDashFeedback(Direction direction)
	{
		_dashFeedbackDirection = direction;
		_lastDashFeedbackAt.restart();
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
		return "scenario " + _snapshot->scenario.id + " | tick " + QString::number(_snapshot->tick) +
		       " | team " + team + " | HP " + hp +
		       " | attack " + attack + " | dash " + dash + " | score B/R " + blueScore + "/" + redScore +
		       " | objective " + _snapshot->objective.state + " | snapshot age " + age + "ms";
	}

	void ArenaView::paintEvent(QPaintEvent*)
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.fillRect(rect(), QColor{17, 20, 22});

		if (!_snapshot.has_value())
		{
			painter.setPen(QColor{247, 243, 232});
			painter.drawText(rect(), Qt::AlignCenter, "No server snapshot yet");
			return;
		}

		drawBoard(painter);
		drawBases(painter);
		drawHazards(painter);
		drawObjective(painter);
		drawLocalActionPreview(painter);
		drawPlayers(painter);
		if (_snapshot->finished)
		{
			drawMatchOverOverlay(painter);
		}
		drawHazardLegend(painter);
	}

	void ArenaView::mouseMoveEvent(QMouseEvent* event)
	{
		_aimDirection = directionFromPoint(mousePoint(event));
		emit aimChanged(_aimDirection);
		update();
	}

	void ArenaView::mousePressEvent(QMouseEvent* event)
	{
		_aimDirection = directionFromPoint(mousePoint(event));
		emit aimChanged(_aimDirection);
		if (event->button() == Qt::LeftButton)
		{
			emit intentRequested(ClientIntent{ClientIntentKind::Attack, _aimDirection});
		}
		if (event->button() == Qt::RightButton)
		{
			emit intentRequested(ClientIntent{ClientIntentKind::Dash, _aimDirection});
		}
	}

	Team ArenaView::viewerTeam() const
	{
		const auto player = localPlayer();
		return player.has_value() ? player->team : Team::Blue;
	}

	QRectF ArenaView::boardRect() const
	{
		const QRectF available = rect().adjusted(10, 10, -10, -10);
		if (!_snapshot.has_value())
		{
			return available;
		}
		const auto ratio = std::min(_snapshot->map.width / _snapshot->map.height, 1.28);
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
		painter.setPen(QPen{QColor{189, 167, 105}, 2});
		painter.setBrush(QColor{26, 31, 32});
		painter.drawRoundedRect(board, 5, 5);

		const auto cellW = board.width() / _snapshot->map.width;
		const auto cellH = board.height() / _snapshot->map.height;
		painter.setPen(QPen{QColor{238, 214, 160, 24}, 1});
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
		const auto viewer = viewerTeam();
		const auto enemy = viewer == Team::Red ? Team::Blue : Team::Red;
		auto translucent = [](QColor color) {
			color.setAlpha(48);
			return color;
		};
		painter.setPen(Qt::NoPen);
		painter.setBrush(translucent(teamColor(viewer)));
		painter.drawRect(QRectF{board.left(), board.bottom() - band, board.width(), band});
		painter.setBrush(translucent(teamColor(enemy)));
		painter.drawRect(QRectF{board.left(), board.top(), board.width(), band});
		painter.setPen(QColor{247, 243, 232, 205});
		painter.drawText(QRectF{board.left(), board.bottom() - band, board.width(), band}, Qt::AlignCenter, "OWN BASE");
		painter.drawText(QRectF{board.left(), board.top(), board.width(), band}, Qt::AlignCenter, "ENEMY BASE");
	}

	void ArenaView::drawObjective(QPainter& painter)
	{
		const auto center = toScreen(_snapshot->objective.x, _snapshot->objective.y);
		const bool carried = !_snapshot->objective.carrierPlayerId.isEmpty() && _snapshot->objective.carrierPlayerId != "0";
		if (!carried)
		{
			const QColor objectiveGlow = _snapshot->objective.state == "dropped" ? QColor{255, 132, 94, 205}
			                         : _snapshot->objective.state == "captured" ? QColor{125, 224, 158, 205}
			                                                                  : QColor{255, 224, 113, 180};
			painter.setPen(QPen{objectiveGlow, 4});
			painter.setBrush(Qt::NoBrush);
			painter.drawEllipse(center, 21, 21);
			painter.setPen(QPen{QColor{255, 255, 255}, 2});
			painter.setBrush(QColor{255, 200, 87});
			QPolygonF diamond;
			diamond << QPointF{center.x(), center.y() - 14} << QPointF{center.x() + 14, center.y()}
			        << QPointF{center.x(), center.y() + 14} << QPointF{center.x() - 14, center.y()};
			painter.drawPolygon(diamond);
			painter.setPen(QColor{245, 248, 250});
			painter.drawText(QRectF{center.x() - 46, center.y() - 34, 92, 20}, Qt::AlignCenter,
			                 _snapshot->objective.state.toUpper());
			if (_snapshot->objective.pickupLockTicks > 0)
			{
				painter.setPen(QColor{255, 235, 190});
				painter.drawText(QRectF{center.x() - 48, center.y() + 18, 96, 18}, Qt::AlignCenter,
				                 "LOCK " + QString::number(_snapshot->objective.pickupLockTicks));
			}
		}
	}

	void ArenaView::drawHazards(QPainter& painter)
	{
		const auto board = boardRect();
		const auto cell = std::min(board.width() / _snapshot->map.width, board.height() / _snapshot->map.height);
		for (const auto& hazard : _snapshot->hazards)
		{
			const auto center = toScreen(hazard.x, hazard.y);
			const auto icon = hazardIconKind(hazard);
			const auto reach = (hazard.trigger == "range" ? hazard.range : hazard.radius) * cell;
			const auto rangeColor =
			    hazard.effect == "damage_drop_objective" ? QColor{255, 207, 92, 44} : QColor{255, 141, 122, 44};
			painter.setBrush(rangeColor);
			painter.setPen(QPen{rangeColor.darker(115), 1});
			painter.drawEllipse(center, std::max(reach, cell * 0.28), std::max(reach, cell * 0.28));
			if (icon == "tower")
			{
				painter.setBrush(hazard.triggered ? QColor{156, 87, 255} : QColor{120, 96, 180});
				painter.setPen(QPen{QColor{230, 220, 255}, 1});
				painter.drawRect(QRectF{center.x() - 9, center.y() - 9, 18, 18});
			}
			else if (icon == "crow")
			{
				painter.setBrush(hazard.triggered ? QColor{235, 246, 255} : QColor{176, 196, 208});
				painter.setPen(QPen{QColor{35, 48, 56}, 1});
				painter.drawEllipse(center, 9, 7);
				painter.setBrush(QColor{35, 48, 56});
				QPolygonF wings;
				wings << QPointF{center.x() - 13, center.y()} << QPointF{center.x(), center.y() - 5}
				      << QPointF{center.x() + 13, center.y()} << QPointF{center.x(), center.y() + 4};
				painter.drawPolygon(wings);
			}
			else
			{
				painter.setBrush(hazard.triggered ? QColor{255, 155, 90} : QColor{255, 159, 67});
				painter.setPen(QPen{QColor{255, 230, 210}, 1});
				QPolygonF mine;
				mine << QPointF{center.x(), center.y() - 10} << QPointF{center.x() + 10, center.y() + 8}
				     << QPointF{center.x() - 10, center.y() + 8};
				painter.drawPolygon(mine);
			}
			if (hazard.cooldown > 0)
			{
				painter.setPen(QColor{248, 251, 255});
				painter.drawText(QRectF{center.x() - 18, center.y() + 12, 36, 14}, Qt::AlignCenter,
				                 QString::number(hazard.cooldown));
			}
		}
	}

	void ArenaView::drawHazardLegend(QPainter& painter)
	{
		QStringList lines;
		QStringList seen;
		for (const auto& hazard : _snapshot->hazards)
		{
			if (seen.contains(hazard.icon))
			{
				continue;
			}
			seen.push_back(hazard.icon);
			const auto label = hazardIconKind(hazard);
			const auto effect = hazard.effect == "damage_drop_objective" ? QStringLiteral("damage + drop")
			                                                              : QStringLiteral("damage");
			const auto reach = hazard.trigger == "range" ? hazard.range : hazard.radius;
			lines.push_back(label + ": " + effect + ", r" + QString::number(reach));
		}
		if (lines.empty())
		{
			return;
		}
		const QRectF panel{static_cast<double>(width() - 240), 14.0, 220.0, 20.0 + 17.0 * lines.size()};
		painter.setBrush(QColor{0, 0, 0, 115});
		painter.setPen(Qt::NoPen);
		painter.drawRect(panel);
		painter.setPen(QColor{248, 251, 255});
		for (int index = 0; index < lines.size(); ++index)
		{
			painter.drawText(QRectF{panel.x() + 10, panel.y() + 8 + 17 * index, panel.width() - 20, 16},
			                 Qt::AlignLeft | Qt::AlignVCenter, lines[index]);
		}
	}

	void ArenaView::drawLocalActionPreview(QPainter& painter)
	{
		const auto player = localPlayer();
		if (!player.has_value())
		{
			return;
		}

		const auto center = toScreen(player->x, player->y);
		const auto board = boardRect();
		const auto range = std::min(board.width() / _snapshot->map.width, board.height() / _snapshot->map.height) * 2.3;
		painter.setBrush(Qt::NoBrush);
		painter.setPen(QPen{QColor{255, 255, 255, 42}, 1});
		painter.drawEllipse(center, range, range);
		painter.setPen(QPen{QColor{255, 245, 170, 145}, 2});
		painter.drawLine(center, directionEnd(center, _aimDirection, range));

		if (_lastAttackFeedbackAt.isValid() && _lastAttackFeedbackAt.elapsed() < 420)
		{
			painter.setPen(QPen{QColor{255, 228, 92, 230}, 4});
			painter.drawLine(center, directionEnd(center, _attackFeedbackDirection, range * 1.1));
		}
		if (_lastDashFeedbackAt.isValid() && _lastDashFeedbackAt.elapsed() < 520)
		{
			painter.setPen(QPen{QColor{112, 222, 255, 210}, 5});
			painter.drawLine(center, directionEnd(center, _dashFeedbackDirection, range * 0.9));
		}
	}

	void ArenaView::drawPlayers(QPainter& painter)
	{
		const auto board = boardRect();
		const auto cell = std::min(board.width() / _snapshot->map.width, board.height() / _snapshot->map.height);
		const auto spriteSize = std::clamp(cell * 0.98, 28.0, 56.0);
		for (const auto& player : _snapshot->players)
		{
			const bool isLocal = player.playerId == _localPlayerId;
			const bool carriesObjective = player.playerId == _snapshot->objective.carrierPlayerId;
			const auto center = toScreen(player.x, player.y);
			const auto teamAccent = isLocal ? playerColor(true) : teamColor(player.team);
			if (carriesObjective)
			{
				painter.setBrush(Qt::NoBrush);
				painter.setPen(QPen{QColor{255, 224, 88, 220}, 5});
				painter.drawEllipse(center, spriteSize * 0.64, spriteSize * 0.64);
			}
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen{teamAccent, isLocal ? 4.5 : 3.5});
			painter.drawEllipse(center, spriteSize * 0.55, spriteSize * 0.55);
			drawPlayerSprite(painter, center, spriteSize, isLocal ? _aimDirection : Direction{0, -1}, isLocal);
			if (isLocal)
			{
				painter.setBrush(playerColor(true));
				painter.setPen(QPen{QColor{255, 255, 255}, 1});
				painter.drawEllipse(directionEnd(center, _aimDirection, spriteSize * 0.42), 4.0, 4.0);
			}
			painter.setBrush(Qt::NoBrush);
			painter.setPen(QPen{QColor{86, 216, 122}, 3});
			const auto hpSpan = std::clamp(player.hp, 0, 100) * 57;
			painter.drawArc(QRectF{center.x() - spriteSize * 0.58, center.y() - spriteSize * 0.58, spriteSize * 1.16,
			                       spriteSize * 1.16},
			                90 * 16, -hpSpan);
			if (carriesObjective)
			{
				painter.setPen(QPen{QColor{255, 255, 255}, 1});
				painter.setBrush(QColor{255, 214, 64});
				QPolygonF marker;
				marker << QPointF{center.x(), center.y() - spriteSize * 0.78}
				       << QPointF{center.x() + 7, center.y() - spriteSize * 0.56}
				       << QPointF{center.x(), center.y() - spriteSize * 0.36}
				       << QPointF{center.x() - 7, center.y() - spriteSize * 0.56};
				painter.drawPolygon(marker);
			}
			painter.setPen(QColor{230, 235, 240});
			painter.drawText(QRectF{center.x() - 34, center.y() + spriteSize * 0.44, 68, 18}, Qt::AlignCenter,
			                 isLocal ? "YOU" : teamName(player.team).toUpper());
		}
	}

	void ArenaView::drawMatchOverOverlay(QPainter& painter)
	{
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
		const auto board = boardRect();
		painter.setBrush(QColor{0, 0, 0, 150});
		painter.setPen(Qt::NoPen);
		painter.drawRect(board);
		painter.setPen(QColor{248, 251, 255});
		QFont font = painter.font();
		font.setPointSize(24);
		font.setBold(true);
		painter.setFont(font);
		painter.drawText(QRectF{board.left(), board.center().y() - 42, board.width(), 36}, Qt::AlignCenter, "MATCH OVER");
		font.setPointSize(15);
		font.setBold(false);
		painter.setFont(font);
		painter.drawText(QRectF{board.left(), board.center().y(), board.width(), 28}, Qt::AlignCenter,
		                 "Blue " + blueScore + " - " + redScore + " Red");
	}

	void ArenaView::drawPlayerSprite(QPainter& painter, QPointF center, double size, Direction facing, bool isLocal)
	{
		if (!_playerSprite.isValid())
		{
			painter.setBrush(playerColor(isLocal));
			painter.setPen(QPen{QColor{255, 255, 255}, 2});
			painter.drawEllipse(center, size * 0.35, size * 0.35);
			return;
		}

		painter.save();
		painter.translate(center);
		painter.rotate(directionAngle(facing));
		const QRectF target{-size / 2.0, -size / 2.0, size, size};
		_playerSprite.render(&painter, target);
		painter.restore();
	}

	QPointF ArenaView::directionEnd(QPointF origin, Direction direction, double length) const
	{
		double dx = static_cast<double>(direction.dx);
		double dy = static_cast<double>(direction.dy);
		if (dx == 0.0 && dy == 0.0)
		{
			dy = -1.0;
		}
		const auto magnitude = std::sqrt((dx * dx) + (dy * dy));
		return QPointF{origin.x() + (dx / magnitude) * length, origin.y() + (dy / magnitude) * length};
	}

	double ArenaView::directionAngle(Direction direction) const
	{
		double dx = static_cast<double>(direction.dx);
		double dy = static_cast<double>(direction.dy);
		if (dx == 0.0 && dy == 0.0)
		{
			dy = -1.0;
		}
		constexpr double radiansToDegrees = 57.29577951308232;
		return std::atan2(dx, -dy) * radiansToDegrees;
	}
}
