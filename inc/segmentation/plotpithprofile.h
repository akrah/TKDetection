#ifndef PLOTKNOTPITHPROFILE_H
#define PLOTKNOTPITHPROFILE_H

#include <qwt_plot_histogram.h>

class PithProfile;
class QwtPlot;

class PlotPithProfile
{
public:
	PlotPithProfile();
	~PlotPithProfile();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );

	void update( const PithProfile & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTPITHPROFILE_H
