#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[]) {
	const QApplication a(argc, argv);

	MainWindow w;
	w.show();

	return a.exec();
}
