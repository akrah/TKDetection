#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include "sliceview_def.h"
#include "marrowextractor_def.h"

namespace Ui {
	class MainWindow;
}

class Billon;
class SliceView;
class SliceHistogram;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow( QWidget *parent = 0 );
	~MainWindow();

	bool eventFilter( QObject *obj, QEvent *event );

private slots:
	void openDicom();
	void closeImage();
	void adaptToSliceType( const SliceType::SliceType &type );
	void newMarrow();
	void updateHistogram();

private:
	void updateComponentsState();
	void newBillon(const QString &folderName = "");

private:
	Ui::MainWindow *ui;

	Billon *_billon;
	SliceView *_sliceView;
	SliceHistogram *_sliceHistogram;
	Marrow *_marrow;

	unsigned int _currentSlice;

	QButtonGroup _groupSliceView;
};

#endif // MAINWINDOW_H
