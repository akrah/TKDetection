#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include "billon_def.h"
#include "marrow_def.h"
#include "piechart.h"
#include "pointpolarseriesdata.h"

#include <QList>
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>
#include <qwt_polar_curve.h>

class Marrow;
class PieChart;
class QwtPlot;
class QwtPolarPlot;
class QwtIntervalSample;
class Interval;

class PieChartDiagrams
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	int count() const;
	const QVector<Interval> &branchesSectors() const;

	void attach( QwtPolarPlot * const polarPlot );
	void attach( QwtPlot * const plot );
	void clearAll();

	void setMovementsThresholdMin( const int &threshold );
	void setMovementsThresholdMax( const int &threshold );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMarrowAroundDiameter( const int &diameter );
	void enableSmoothing( const bool &enable );

	void compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const Interval &slicesInterval, const Interval &intensity );
	void highlightCurve( const int &index );

	void draw( QImage &image, const iCoord2D &center ) const;

private:
	void computeValues();
	void computeMaximums();
	void computeIntervals();

private:
	QVector<qreal> _datas;
	QVector<int> _maximums;
	QVector<Interval> _intervals;

	QwtPolarCurve _curve;
	PointPolarSeriesData *_datasCurve;
	QwtPlotHistogram _histogram;

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
	qreal _dataPercentage;

	PieChart _pieChart;

	int _movementsThresholdMin;
	int _movementsThresholdMax;
	bool _useNextSlice;
	int _marrowAroundDiameter;
	bool _smoothing;

};

#endif // PIECHARTDIAGRAMS_H
