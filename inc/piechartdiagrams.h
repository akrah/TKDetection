#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include "billon_def.h"
#include "inc/pointpolarseriesdata.h"

#include <QList>
#include <qwt_plot_histogram.h>
#include <qwt_polar_curve.h>

class Marrow;
class PieChart;
class SlicesInterval;
class IntensityInterval;
class QwtPlot;
class QwtPolarPlot;
class QwtIntervalSample;

class PieChartDiagrams
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	int count() const;

	void attach( QwtPolarPlot * const polarPlot );
	void attach( QwtPlot * const plot );
	void detach();
	void clearAll();

	void setMovementsThresholdMin( const int &threshold );
	void setMovementsThresholdMax( const int &threshold );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMarrowArroundDiameter( const int &diameter );

	void compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity );
	void highlightCurve( const int &index );

private:
	void createDiagrams( const QVector<int> &sectorsSum, const int &nbSectors );
	void updateMaximums();
	int sliceOfIemeMaximum( const int &maximumIndex ) const;
	void computeIntervals();

private:
	QwtPolarCurve _curve;
	PointPolarSeriesData *_curveDatas;
	QwtPolarCurve _curveMaximums;
	PointPolarSeriesData *_curveMaximumsDatas;
	QwtPolarCurve _highlightCurve;
	PointPolarSeriesData *_highlightCurveDatas;

	QwtPlotHistogram _curveHistogram;
	QVector<QwtIntervalSample> _curveHistogramDatas;

	QwtPlotHistogram _curveHistogramMaximums;
	QVector<QwtIntervalSample> _curveHistogramMaximumsDatas;

	QwtPlotHistogram _curveHistogramIntervals;
	QVector<QwtIntervalSample> _curveHistogramIntervalsDatas;
	QVector<QwtInterval> _curveHistogramIntervalsRealDatas;

	int _movementsThresholdMin;
	int _movementsThresholdMax;
	bool _useNextSlice;
	int _marrowAroundDiameter;
};

#endif // PIECHARTDIAGRAMS_H
