#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include"intervalscomputerdefaultparameters.h"
#include <QVector>

class Interval;

class Histogram
{
public:
	Histogram();
	virtual ~Histogram();

	int count() const;
	qreal value( const uint &index ) const;
	int nbMaximums() const;
	int indexOfIemeMaximum( const int &maximumIndex ) const;
	int indexOfIemeInterval( const int &intervalIndex ) const;
	int marrowAroundDiameter() const;

	void setMarrowAroundDiameter( const int &diameter );
	void setSmoothingType( const SmoothingType::SmoothingType &type );
	void setMaximumsNeighborhood( const int &neighborhood );
	void setMinimumIntervalWidth( const int &width );
	void setMovementThresholdMin( const int &threshold );
	void setMovementThresholdMax( const int &threshold );
	void setDerivativePercentage( const qreal &percentage );

	void useNextSliceInsteadOfCurrentSlice( const bool &enable );

protected:
	QVector<qreal> _datas;
	QVector<int> _maximums;
	QVector<Interval> _intervals;

	int _marrowAroundDiameter;
	SmoothingType::SmoothingType _smoothing;
	int _maximumsNeighborhood;
	int _minimumIntervalWidth;
	int _movementThresholdMin;
	int _movementThresholdMax;
	qreal _derivativePercentage;

	qreal _derivativeThreshold;

	bool _useNextSlice;
};

#endif // HISTOGRAM_H
