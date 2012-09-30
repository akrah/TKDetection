#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include "histogram.h"
#include "billon_def.h"
#include "marrow_def.h"
#include "piechart.h"
#include "pointpolarseriesdata.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>
#include <qwt_polar_curve.h>

class Marrow;
class PieChart;
class QwtPlot;
class QwtPolarPlot;
class QwtIntervalSample;
template<typename T> class Interval;

class PieChartDiagrams : public Histogram<qreal>
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	const QVector< Interval<int> > &knotIntervals() const;

	void attach( QwtPolarPlot * const polarPlot );
	void attach( QwtPlot * const plot );
	void clear();

	void compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart,
				  const Interval<int> &slicesInterval, const Interval<int> &intensity, const Interval<int> &motionInterval,
				  const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum, const int &maximumsNeighborhood,
				  const int &derivativePercentage, const int &minimumIntervalWidth, const int &radiusAroundPith );
	void highlightCurve( const int &index );

	void draw( QImage &image, const iCoord2D &center ) const;

private:
	void computeValues();
	void computeMaximums();
	void computeIntervals();

private:
	QwtPolarCurve _curve;
	PointPolarSeriesData *_datasCurve;
	QwtPlotHistogram _plotHistogram;

	QwtPolarCurve _curveMaximums;
	PointPolarSeriesData *_datasCurveMaximums;
	QwtPlotHistogram _histogramMaximums;

	QwtPolarCurve _curveIntervals;
	PointPolarSeriesData *_datasCurveIntervals;
	QwtPlotHistogram _histogramIntervals;

	QwtPolarCurve _highlightCurve;
	PointPolarSeriesData *_highlightCurveDatas;
	QwtPlotHistogram _highlightCurveHistogram;

	QwtPlotCurve _curvePercentage;

	PieChart _pieChart;
};

#endif // PIECHARTDIAGRAMS_H
