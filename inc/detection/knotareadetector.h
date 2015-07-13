#ifndef KNOTAREADETECTOR_H
#define KNOTAREADETECTOR_H

#include "def/def_billon.h"
#include "inc/interval.h"

#include <QVector>
#include <QRect>

class KnotAreaDetector
{
public:
	KnotAreaDetector();
	virtual ~KnotAreaDetector();

	virtual void execute( const Billon &billon ) = 0;
	virtual void clear();

	const QVector<QRect> &knotAreas() const;

	const Interval<int> &intensityInterval() const;
	void setIntensityInterval( const Interval<int> interval );

	const uint &zMotionMin() const;
	void setZMotionMin( const uint &value );

	const qreal &treeRadius() const;
	void setTreeRadius( const qreal &radius );


protected:
	QVector<QRect> _knotAreas;
	Interval<int> _intensityInterval;
	uint _zMotionMin;
	qreal _treeRadius;
};

#endif // KNOTAREADETECTOR_H
