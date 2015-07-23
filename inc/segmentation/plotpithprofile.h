#ifndef PLOTKNOTPITHPROFILE_H
#define PLOTKNOTPITHPROFILE_H

#include "inc/plothistogram.h"

#include <qwt_plot_histogram.h>

class PlotPithProfile : public PlotHistogram
{
public:
	PlotPithProfile();
	~PlotPithProfile();

	void attach( QwtPlot * const plot );
	void clear();

	void update( const Histogram<qreal> &histogram );

	void moveCursor( const uint & sliceIndex );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTPITHPROFILE_H
