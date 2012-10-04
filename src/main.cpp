#include <QApplication>
#include <QTextCodec>

#include "inc/mainwindow.h"
#include <QMessageBox>
#include "tst/test_intervalshistogram.h"

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
	qInstallMsgHandler(outputHandler);

	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	return a.exec();

//	Test_IntervalsHistogram::allTests();
//	return 0;
}
