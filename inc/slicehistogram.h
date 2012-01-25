#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QVector>

class Billon;
class QwtPlotHistogram;
class QwtIntervalSample;
class QwtPlot;

class SliceHistogram
{

public:
	SliceHistogram();
	~SliceHistogram();

	qreal value( const int &index );

	void setModel( const Billon *billon );

	void attach( QwtPlot * const plot );
	void detach();

	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );

	void constructHistogram();

private:
	const Billon *_billon;

	QwtPlotHistogram *_histogram;
	QVector<QwtIntervalSample> _datas;
	QList<int> _pics;

	int _lowThreshold;
	int _highThreshold;
};

#endif // SLICEHISTOGRAM_H
