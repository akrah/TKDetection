#ifndef PLOTKNOTPITHPROFILE_H
#define PLOTKNOTPITHPROFILE_H

#include <qwt_plot_histogram.h>

class KnotPithProfile;
class QwtPlot;

class PlotKnotPithProfile
{
public:
	PlotKnotPithProfile();
	~PlotKnotPithProfile();

	void attach( QwtPlot * const plot );
	void clear();

	void moveCursor( const uint & sliceIndex );

	void update( const KnotPithProfile & histogram );

private:
	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramCursor;
};

#endif // PLOTKNOTPITHPROFILE_H
