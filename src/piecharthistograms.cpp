#include "inc/piecharthistograms.h"

#include "inc/piechart.h"
#include <qwt_plot.h>

PieChartHistograms::PieChartHistograms() : _slice(0), _pieChart(0) {
}

void PieChartHistograms::setModel( const imat * slice ) {
	_slice = slice;
}

void PieChartHistograms::setModel( const PieChart * pieChart ) {
	_pieChart = pieChart;
}

const QList<QwtPlot> &PieChartHistograms::histograms() {
	return _histograms;
}

void PieChartHistograms::computeHistograms() {
	// TODO : parcour de la slice et calcul de l'histogram
}
