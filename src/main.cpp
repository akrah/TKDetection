#include <QtGui/QApplication>
#include <QTextCodec>

#include "inc/mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	MainWindow w;
	w.show();

	return a.exec();
}
