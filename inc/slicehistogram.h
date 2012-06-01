#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QVector>
#include "billon_def.h"

#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

class Marrow;
class QwtPlot;
class QwtIntervalSample;
class QwtInterval;
class Interval;

class SliceHistogram
{

public:
	SliceHistogram();
	~SliceHistogram();

	qreal value( const int &index ) const;
	int nbMaximums() const;
	int sliceOfIemeMaximum( const int &maximumIndex ) const;
	int sliceOfIemeInterval( const int &intervalIndex ) const;
	int marrowAroundDiameter() const;
	const QVector<Interval> &branchesAreas() const;

	void setMarrowAroundDiameter( const int &diameter );
	void setMinimumIntervalWidth( const int &width );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void enableSmoothing( const bool &enable );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMaximumsNeighborhood( const int &neighborhood );
	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const Marrow *marrow, const Interval &intensity );

private:
	void computeValues();
	void computeMaximums();
	void computeIntervals();

private:
	QVector<qreal> _datas;
	QVector<int> _maximums;
	QVector<Interval> _intervals;

	QwtPlotHistogram _histogramData;
	QwtPlotHistogram _histogramMaximums;
	QwtPlotHistogram _histogramIntervals;

	QwtPlotCurve _curvePercentage;
	qreal _dataPercentage;

	int _marrowAroundDiameter;
	int _minimumIntervalWidth;
	int _movementThresholdMin;
	int _movementThresholdMax;
	bool _smoothing;
	bool _useNextSlice;
	int _maximumsNeighborhood;
};

#endif // SLICEHISTOGRAM_H
