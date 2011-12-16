#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QWidget>
#include <vector>

class Billon;

#define HISTOGRAM_HEIGHT 200

class SliceHistogram : public QWidget
{
	Q_OBJECT

public:
	explicit SliceHistogram( QWidget *parent = 0 );

	void setModel(const Billon *billon);
	void update();

protected:
	void paintEvent(QPaintEvent *event);

private:
	const Billon *_billon;
	std::vector<float> _graph;
};

#endif // SLICEHISTOGRAM_H
