#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <qwt_plot_histogram.h>

class Billon;
class QwtIntervalSample;

class SliceHistogram : public QObject, public QwtPlotHistogram
{
	Q_OBJECT

public:
	explicit SliceHistogram( QwtPlot* parent = 0 );

	void setModel( const Billon *billon );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );

	void constructHistogram();

private:
	const Billon *_billon;
	QVector<QwtIntervalSample> _datas;

	int _lowThreshold;
	int _highThreshold;
};

#endif // SLICEHISTOGRAM_H
