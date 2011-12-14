#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
	class MainWindow;
}

class Billon;
class SliceHistogram;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	void openDicom(const QString &);
	void drawAverageSlice();

	void enableImageModifiers(bool);

private slots:
	void openDicom();
	void closeImage();
	void updateImage();
	void setImage(const int);
	void changeAverageSliceVisibility(bool);

private:
	Ui::MainWindow *ui;

	Billon *_cube;
	QImage *_image;
	unsigned int _currentSlice;

	SliceHistogram *_sliceHistogram;
};

#endif // MAINWINDOW_H
