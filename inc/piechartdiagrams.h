#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include "billon_def.h"
#include "slicehistogram_def.h"
#include "marrow_def.h"
#include "piechart.h"

#include <QList>
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>
#include <qwt_polar_curve.h>

class Marrow;
class PieChart;
class SlicesInterval;
class IntensityInterval;
class QwtPlot;
class QwtPolarPlot;
class QwtIntervalSample;
class PointPolarSeriesData;

class PieChartDiagrams
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	int count() const;

	void attach( QwtPolarPlot * const polarPlot );
	void attach( QwtPlot * const plot );

	void setMovementsThresholdMin( const int &threshold );
	void setMovementsThresholdMax( const int &threshold );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMarrowArroundDiameter( const int &diameter );
	void setIntervalType( const HistogramIntervalType::HistogramIntervalType &type );

	void compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity );
	void highlightCurve( const int &index );

	void draw( QImage &image, const iCoord2D &center ) const;

private:
	int sliceOfIemeMaximum( const int &maximumIndex ) const;
	void createDiagrams( const QVector<int> &sectorsSum );
	void computeMeansAndMedian();
	void computeMaximums();
	void computeIntervals();

private:
	QwtPolarCurve _curve;
	PointPolarSeriesData *_curveDatas;
	QwtPlotHistogram _curveHistogram;
	QVector<QwtIntervalSample> _curveHistogramDatas;

	QwtPolarCurve _curveMaximums;
	PointPolarSeriesData *_curveMaximumsDatas;
	QwtPlotHistogram _curveHistogramMaximums;
	QVector<QwtIntervalSample> _curveHistogramMaximumsDatas;
	QVector<int> _maximumsIndex;

	QwtPolarCurve _highlightCurve;
	PointPolarSeriesData *_highlightCurveDatas;
	QwtPlotHistogram _highlightCurveHistogram;
	QVector<QwtIntervalSample> _highlightCurveHistogramDatas;

	QwtPolarCurve _curveIntervals;
	PointPolarSeriesData *_curveIntervalsDatas;
	QwtPlotHistogram _curveHistogramIntervals;
	QVector<QwtIntervalSample> _curveHistogramIntervalsDatas;
	QVector<QwtInterval> _curveHistogramIntervalsRealDatas;

	PieChart _pieChart;

	int _movementsThresholdMin;
	int _movementsThresholdMax;
	bool _useNextSlice;
	int _marrowAroundDiameter;
	HistogramIntervalType::HistogramIntervalType _intervalType;

	qreal _dataMeans;
	QwtPlotCurve _curveMeans;
	qreal _dataMedian;
	QwtPlotCurve _curveMedian;
	qreal _dataMeansMedian;
	QwtPlotCurve _curveMeansMedian;
};

#endif // PIECHARTDIAGRAMS_H
