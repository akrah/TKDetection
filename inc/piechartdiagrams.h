#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include "billon_def.h"

#include <QList>
#include <qwt_polar_curve.h>

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

	void attach( const QList<QwtPlot *> & plots );
	void attach( QwtPolarPlot * const polarPlot );
	void detach();

	void setMovementsThresholdMin( const int &threshold );
	void setMovementsThresholdMax( const int &threshold );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMarrowArroundDiameter( const int &diameter );

	void compute( const Billon &billon, const Marrow *marrow, const PieChart &pieChart, const SlicesInterval &slicesInterval, const IntensityInterval &intensity );
	void highlightCurve( const int &index );

private:
	void clearAll();
	void initializeDiagramsData( QVector< QVector<QwtIntervalSample> > &histogramsData, const int &nbSectors, const int &nbValues );
	void createDiagrams( QVector< QVector<QwtIntervalSample> > &histogramsData, const QVector<int> &sectorsSum, const int &nbSectors );

private:
	QList<QwtPlotHistogram *> _histograms;
	QwtPolarCurve *_polarCurve;
	QwtPolarCurve _highlightCurve;

	int _movementsThresholdMin;
	int _movementsThresholdMax;
	bool _useNextSlice;
	int _marrowAroundDiameter;
};

#endif // PIECHARTDIAGRAMS_H
