#include <QApplication>
#include "gamewindowtest.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	GameWindow *window = new GameWindow();
	window->show();
	return a.exec();
}
