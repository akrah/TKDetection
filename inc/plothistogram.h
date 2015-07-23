#ifndef PLOTHISTOGRAM_H
#define PLOTHISTOGRAM_H

#include "inc/histogram.h"

class QwtPlot;

class PlotHistogram
{
public:
	PlotHistogram();
	virtual ~PlotHistogram();

	virtual void attach( QwtPlot * const plot ) = 0;
	virtual void clear() = 0;

	virtual void update( const Histogram<qreal> &histogram ) = 0;

	virtual void moveCursor( const uint &index ) = 0;
};

#endif // PLOTHISTOGRAM_H
