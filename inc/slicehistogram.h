#ifndef SLICEHISTOGRAM_H
#define SLICEHISTOGRAM_H

#include <QVector>

#include "slicehistogram_def.h"
#include "billon_def.h"
class Marrow;
class QwtPlot;
class QwtPlotHistogram;
class QwtPlotCurve;
class QwtIntervalSample;
class QwtInterval;
class IntensityInterval;

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
	const QVector<QwtInterval> & branchesAreas() const;

	void setMarrowAroundDiameter( const int &diameter );
	void setIntervalType( const HistogramIntervalType::HistogramIntervalType &type );
	void setMinimumIntervalWidth( const int &width );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void enableSmoothing( const bool &enable );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );
	void setMaximumsNeighborhood( const int &neighborhood );
	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon, const Marrow *marrow, const IntensityInterval &intensity );

private:
	void updateMaximums();
	void computeIntervals();
	void computeMeansAndMedian();

	void smoothHistogram( QVector< QwtIntervalSample > &histogramDatas );

private:
	QwtPlotHistogram *_histogram;
	QVector<QwtIntervalSample> _datasHistogram;

	QwtPlotHistogram *_histogramMaximums;
	QVector<QwtIntervalSample> _datasMaximums;

	QwtPlotHistogram *_histogramBranchesArea;
	QVector<QwtIntervalSample> _datasBranchesAreaToDrawing;
	QVector<QwtInterval> _datasBranchesRealAreas;

	QwtPlotCurve *_curveMeans;
	qreal _dataMeans;

	QwtPlotCurve *_curveMedian;
	qreal _dataMedian;

	QwtPlotCurve *_curveMeansMedian;
	qreal _dataMeansMedian;

	int _marrowAroundDiameter;
	HistogramIntervalType::HistogramIntervalType _intervalType;
	int _minimumIntervalWidth;
	int _movementThresholdMin;
	int _movementThresholdMax;
	bool _smoothing;
	bool _useNextSlice;
	int _maximumsNeighborhood;
};

#endif // SLICEHISTOGRAM_H
