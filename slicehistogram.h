#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QWidget>

class Billon;

#define HISTOGRAM_HEIGHT 200

class SliceHistogram : public QWidget
{
	Q_OBJECT

public:
	explicit SliceHistogram(QWidget *parent = 0);

	void update(const Billon &);

protected:
	void paintEvent(QPaintEvent *);

private:
	QList<uint> _graph;
};

#endif // SLICEHISTOGRAM_H
