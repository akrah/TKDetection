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

class SliceHistogram : public Histogram
{

public:
	SliceHistogram();
	~SliceHistogram();

	const QVector<Interval> &branchesAreas() const;

	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const Marrow *marrow, const Interval &intensity );

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
