#include <QCoreApplication>
#include <QFile>
#include <QStringList>
#include <QSvgRenderer>

#include <iostream>

int main(int argc, char** argv)
{
	QCoreApplication app(argc, argv);
	const QStringList requiredAssets{
	    QStringLiteral(":/if_arena/assets/players/swordsman.svg"),
	    QStringLiteral(":/if_arena/assets/svg/hazard_mine.svg"),
	    QStringLiteral(":/if_arena/assets/svg/hazard_tower.svg"),
	    QStringLiteral(":/if_arena/assets/svg/hazard_crow.svg"),
	    QStringLiteral(":/if_arena/assets/svg/obstacle_block.svg"),
	};

	for (const auto& path : requiredAssets)
	{
		if (!QFile::exists(path))
		{
			std::cerr << "missing Qt SVG resource: " << path.toStdString() << '\n';
			return 1;
		}
		QSvgRenderer renderer(path);
		if (!renderer.isValid())
		{
			std::cerr << "invalid Qt SVG resource: " << path.toStdString() << '\n';
			return 1;
		}
	}
	return 0;
}
