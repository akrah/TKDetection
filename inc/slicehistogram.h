#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QVector>

#include "billon_def.h"
class Marrow;
class QwtPlotHistogram;
class QwtIntervalSample;
class QwtPlot;
class IntensityInterval;

class SliceHistogram
{

public:
	SliceHistogram();
	~SliceHistogram();

	qreal value( const int &index ) const;
	int nbMaximums() const;
	int sliceOfIemeMaximum( const int &maximumIndex ) const;

	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const IntensityInterval &intensityInterval );
	void constructHistogram( const Billon &billon, const Marrow &marrow, const IntensityInterval &intensityInterval );

private:
	void updateMaximums();

private:
	QwtPlotHistogram *_histogram;
	QVector<QwtIntervalSample> _datasHistogram;

	QwtPlotHistogram *_histogramMaximums;
	QVector<QwtIntervalSample> _datasMaximums;
};

#endif // SLICEHISTOGRAM_H
