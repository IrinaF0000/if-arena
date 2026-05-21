#include "ui/MainWindow.hpp"

#include <QApplication>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);
	if_arena::battle_qt_client::ui::MainWindow window;
	window.show();
	return QApplication::exec();
}
