#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include"intervalscomputerdefaultparameters.h"
#include <QVector>
#include "interval.h"

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

	void useNextSliceInsteadOfCurrentSlice( const bool &enable );

protected:
	QVector<qreal> _datas;
	QVector<int> _maximums;
	QVector<Interval> _intervals;

	bool _useNextSlice;
};

#endif // HISTOGRAM_H
