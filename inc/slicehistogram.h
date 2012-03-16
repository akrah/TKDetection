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

	void setModel( const Billon *billon );
	void setModel( const Marrow *marrow );

	void attach( QwtPlot * const plot );
	void detach();

	void constructHistogram( const IntensityInterval &intensityInterval );

private:
	const Billon *_billon;
	const Marrow *_marrow;

	QwtPlotHistogram *_histogram;
	QVector<QwtIntervalSample> _datasHistogram;

	QwtPlotHistogram *_histogramMaximums;
	QVector<QwtIntervalSample> _datasMaximums;
};

#endif // SLICEHISTOGRAM_H
