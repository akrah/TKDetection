#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include <QList>

#include "billon_def.h"
class Marrow;
class PieChart;
class SlicesInterval;
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
	void setModel( const SlicesInterval * const interval );

	void attach( const QList<QwtPlot *> & plots );
	void attach( QwtPolarPlot * const polarPlot );
	void detach();

	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void setBillonInterval( const int &sliceMin, const int &sliceMax );
	void setMinimalDifference( const int &minimalDifference );

	void compute();

private:
	void clearAll();

private:
	const Billon *_billon;
	const Marrow *_marrow;
	const PieChart *_pieChart;
	const SlicesInterval * _slicesInterval;

	int _lowThreshold;
	int _highThreshold;
	int _minimalDifference;

	QList<QwtPlotHistogram *> _histograms;
	QwtPolarCurve *_polarCurve;
};

#endif // PIECHARTDIAGRAMS_H
