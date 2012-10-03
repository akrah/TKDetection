#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include "histogram.h"
#include "def/def_billon.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class Marrow;
class QwtPlot;
template <typename T> class Interval;

class SliceHistogram : public Histogram<qreal>
{

public:
	SliceHistogram();
	virtual ~SliceHistogram();

	const QVector< Interval<int> > & knotAreas() const;

	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const Marrow &marrow, const Interval<int> &intensity, const Interval<int> &motionInterval,
							 const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum, const int &maximumsNeighborhood,
							 const int &derivativePercentage, const int &minimumIntervalWidth, const int &borderPercentageToCut,
							 const int &radiusAroundPith );

private:
	void computeValues();
	void computeMaximums();
	void computeIntervals();

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;

	QwtPlotCurve _curvePercentage;
};

#endif // SLICEHISTOGRAM_H
