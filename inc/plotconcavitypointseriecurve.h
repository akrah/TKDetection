#ifndef PLOTCONCAVITYPOINTSERIECURVE_H
#define PLOTCONCAVITYPOINTSERIECURVE_H

#include <qwt_plot_curve.h>

class ConcavityPointSerieCurve;
template <typename T> class Interval;

class PlotConcavityPointSerieCurve
{
public:
	PlotConcavityPointSerieCurve();
	~PlotConcavityPointSerieCurve();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const ConcavityPointSerieCurve &curve, const Interval<qreal> &angularInterval );

private:
	QwtPlotCurve _maxConcavityPointsData;
	QwtPlotCurve _minConcavityPointsData;
	QwtPlotCurve _maxKnotAreaAngle;
	QwtPlotCurve _minKnotAreaAngle;
};

#endif // PLOTCONCAVITYPOINTSERIECURVE_H
