#ifndef PIECHARTHISTOGRAMS_H
#define PIECHARTHISTOGRAMS_H

#include <QObject>
#include <QList>

class Billon;
class Marrow;
class PieChart;
class QwtPlot;
class QwtPlotHistogram;

class PieChartHistograms : public QObject
{
	Q_OBJECT

public:
	PieChartHistograms();
	~PieChartHistograms();

	int count() const;

	void setModel( const Billon * billon );
	void setModel( const PieChart * pieChart );
	void setModel( const Marrow * marrow );
	void attach( const QList<QwtPlot *> & plots );
	void clear();

private:
	bool intervalIsValid() const;

public slots:
	void setBillonInterval( const int &sliceMin, const int &sliceMax );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void computeHistograms();

signals:
	void histogramsUpdated();

private:
	const Billon *_billon;
	const Marrow *_marrow;
	const PieChart *_pieChart;

	int _beginSlice;
	int _endSlice;
	int _lowThreshold;
	int _highThreshold;

	QList<QwtPlotHistogram *> _histograms;
};

#endif // PIECHARTHISTOGRAMS_H
