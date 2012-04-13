#include <QtGui/QApplication>
#include <QTextCodec>

#include "inc/mainwindow.h"
#include <QMessageBox>

void outputHandler(QtMsgType type, const char *msg)
{
	fprintf(stderr, "%s\n", msg);
	switch (type) {
		case QtWarningMsg:
			QMessageBox::warning(0,QObject::tr("Message d'erreur"),QObject::tr(msg));
			break;
		default:
			break;
	}
}

int main(int argc, char *argv[])
{
//	qInstallMsgHandler(outputHandler);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	return a.exec();
}
