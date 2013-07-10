#ifndef PLOTCONCAVITYPOINTSERIECURVE_H
#define PLOTCONCAVITYPOINTSERIECURVE_H

#include <qwt_plot_curve.h>

class ConcavityPointSerieCurve;

class PlotConcavityPointSerieCurve
{
public:
	PlotConcavityPointSerieCurve();
	~PlotConcavityPointSerieCurve();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const ConcavityPointSerieCurve &curve );

private:
	QwtPlotCurve _leftConcavityPointsData;
	QwtPlotCurve _rightConcavityPointsData;
};

#endif // PLOTCONCAVITYPOINTSERIECURVE_H
