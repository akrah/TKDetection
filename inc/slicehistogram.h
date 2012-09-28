#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QVector>
#include "histogram.h"
#include "billon_def.h"
#include "intervalscomputerdefaultparameters.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class Marrow;
class QwtPlot;
class QwtIntervalSample;
class QwtInterval;
class Interval;

class SliceHistogram : public Histogram<qreal>
{

public:
	SliceHistogram();
	~SliceHistogram();

	const QVector<Interval> &branchesAreas() const;

	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const Marrow *marrow, const Interval &intensity, const Interval &motionInterval,
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
