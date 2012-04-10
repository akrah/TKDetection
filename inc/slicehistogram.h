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
class IntensityInterval;

class SliceHistogram
{

public:
	SliceHistogram();
	~SliceHistogram();

	qreal value( const int &index ) const;
	int nbMaximums() const;
	int sliceOfIemeMaximum( const int &maximumIndex ) const;
	int marrowAroundDiameter() const;

	void setMarrowAroundDiameter( const int &diameter );
	void setIntervalType( const HistogramIntervalType::HistogramIntervalType &type );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void enableSmoothing( const bool &enable );
	void useNextSlice( const bool &enable );
	void attach( QwtPlot * const plot );
	void detach();
	void clear();

	void constructHistogram( const Billon &billon );
	void constructHistogram( const Billon &billon, const Marrow &marrow );

private:
	void updateMaximums();
	void updateMinimums();
	void computeIntervals();
	void computeMeansAndMedian();

	void smoothHistogram( QVector< QwtIntervalSample > &histogramDatas );

private:
	QwtPlotHistogram *_histogram;
	QVector<QwtIntervalSample> _datasHistogram;

	QwtPlotHistogram *_histogramMaximums;
	QVector<QwtIntervalSample> _datasMaximums;

	QwtPlotHistogram *_histogramMinimums;
	QVector<QwtIntervalSample> _datasMinimums;

	QwtPlotHistogram *_histogramBranchesArea;
	QVector<QwtIntervalSample> _datasBranchesArea;

	QwtPlotCurve *_curveMeans;
	qreal _dataMeans;

	QwtPlotCurve *_curveMedian;
	qreal _dataMedian;

	int _marrowAroundDiameter;
	HistogramIntervalType::HistogramIntervalType _intervalType;
	int _movementThresholdMin;
	int _movementThresholdMax;
	bool _smoothing;
	bool _useNextSlice;
};

#endif // SLICEHISTOGRAM_H
