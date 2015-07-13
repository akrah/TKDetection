#include "inc/detection/knotareadetector.h"

#include "inc/define.h"

KnotAreaDetector::KnotAreaDetector() : _intensityInterval(MINIMUM_INTENSITY,MAXIMUM_INTENSITY), _zMotionMin(MINIMUM_Z_MOTION) {}
KnotAreaDetector::~KnotAreaDetector() {}

void KnotAreaDetector::clear()
{
	_knotAreas.clear();
}

const QVector<QRect> &KnotAreaDetector::knotAreas() const
{
	return _knotAreas;
}

const Interval<int> &KnotAreaDetector::intensityInterval() const
{
	return _intensityInterval;
}

void KnotAreaDetector::setIntensityInterval( const Interval<int> interval )
{
	_intensityInterval = interval;
}

const uint &KnotAreaDetector::zMotionMin() const
{
	return _zMotionMin;
}

void KnotAreaDetector::setZMotionMin( const uint &value )
{
	_zMotionMin = value;
}

const qreal &KnotAreaDetector::treeRadius() const
{
	return _treeRadius;
}

void KnotAreaDetector::setTreeRadius( const qreal &radius )
{
	_treeRadius = radius;
}
