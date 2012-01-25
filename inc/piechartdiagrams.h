#ifndef PIECHARTDIAGRAMS_H
#define PIECHARTDIAGRAMS_H

#include <QList>

class Billon;
class Marrow;
class PieChart;
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

	void setModel( const Billon * const billon );
	void setModel( const PieChart * const pieChart );
	void setModel( const Marrow * const marrow );

	void attach( const QList<QwtPlot *> & plots );
	void attach( QwtPolarPlot * const polarPlot );
	void detach();

	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void setBillonInterval( const int &sliceMin, const int &sliceMax );
	void setMinimalDifference( const int &minimalDifference );

	void compute();

private:
	bool intervalIsValid() const;
	void clearAll();

private:
	const Billon *_billon;
	const Marrow *_marrow;
	const PieChart *_pieChart;

	int _beginSlice;
	int _endSlice;
	int _lowThreshold;
	int _highThreshold;
	int _minimalDifference;

	QList<QwtPlotHistogram *> _histograms;
	QwtPolarCurve *_polarCurve;
};

#endif // PIECHARTDIAGRAMS_H
