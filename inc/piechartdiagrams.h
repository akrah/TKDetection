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
class QwtIntervalSample;

class PieChartDiagrams
{
public:
	PieChartDiagrams();
	~PieChartDiagrams();

	int count() const;
	int minimalDifference() const;

	void attach( const QList<QwtPlot *> & plots );
	void attach( QwtPolarPlot * const polarPlot );
	void detach();

	void setMinimalDifference( const int &minimalDifference );

	void compute( const Billon &billon, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensityInterval );
	void compute( const Billon &billon, const PieChart &pieChart, const Marrow &marrow, const SlicesInterval &slicesInterval, const IntensityInterval &intensityInterval );

private:
	void clearAll();
	void initializeDiagramsData( QVector< QVector<QwtIntervalSample> > &histogramsData, const int &nbSectors, const int &nbValues );
	void createDiagrams( QVector< QVector<QwtIntervalSample> > &histogramsData, const QVector<int> &sectorsSum, const int &nbSectors, const int &nbValues, const int &diffMax );

private:
	QList<QwtPlotHistogram *> _histograms;
	QwtPolarCurve *_polarCurve;

	int _minimalDifference;
};

#endif // PIECHARTDIAGRAMS_H
