#ifndef PIECHARTHISTOGRAMS_H
#define PIECHARTHISTOGRAMS_H

#include <QList>

class imat;
class PieChart;
class QwtPlot;

class PieChartHistograms
{
public:
	PieChartHistograms();

	void setModel( const imat * slice );
	void setModel( const PieChart * pieChart );

	const QList<QwtPlot> &histograms();

private:
	void computeHistograms();

private:
	const imat *_slice;
	const PieChart *_pieChart;

	QList<QwtPlot> _histograms;
};

#endif // PIECHARTHISTOGRAMS_H
