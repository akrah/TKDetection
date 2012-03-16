#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include <QList>

#include "billon_def.h"
class Marrow;
class PieChart;
class SlicesInterval;
class IntensityInterval;
class QwtPlot;
class QwtPlotHistogram;
class QwtPolarPlot;
class QwtPolarCurve;

class PieChartDiagrams
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	int count() const;
	int minimalDifference() const;

	void setModel( const Billon * const billon );
	void setModel( const PieChart * const pieChart );
	void setModel( const Marrow * const marrow );

	void attach( const QList<QwtPlot *> & plots );
	void attach( QwtPolarPlot * const polarPlot );
	void detach();

	void setBillonInterval( const int &sliceMin, const int &sliceMax );
	void setMinimalDifference( const int &minimalDifference );

	void compute( const SlicesInterval &slicesInterval, const IntensityInterval &intensityInterval );

private:
	void clearAll();

private:
	const Billon *_billon;
	const Marrow *_marrow;
	const PieChart *_pieChart;

	int _minimalDifference;

	QList<QwtPlotHistogram *> _histograms;
	QwtPolarCurve *_polarCurve;
};

#endif // PIECHARTDIAGRAMS_H
