#include "inc/detection/knotareadetector.h"

#include "inc/define.h"

KnotAreaDetector::KnotAreaDetector() : _pieChart(PieChart(500)), _intensityInterval(MINIMUM_INTENSITY,MAXIMUM_INTENSITY), _zMotionMin(MINIMUM_Z_MOTION), _treeRadius(100) {}
KnotAreaDetector::~KnotAreaDetector() {}

void KnotAreaDetector::clear()
{
	_knotAreas.clear();
}

const QVector<QRect> &KnotAreaDetector::knotAreas() const
{
	return _knotAreas;
}

const QRect &KnotAreaDetector::knotArea( const uint &areaIndex ) const
{
	Q_ASSERT_X( (int)areaIndex<_knotAreas.size(),
				"const QRect &knotArea( const uint &areaIndex )",
				"areaIndex doit être inférieur au nombre de zones de nœuds" );
	return _knotAreas[areaIndex];
}

bool KnotAreaDetector::hasKnotAreas() const
{
	return !_knotAreas.isEmpty();
}

const PieChart &KnotAreaDetector::pieChart() const
{
	return _pieChart;
}

void KnotAreaDetector::setSectorNumber( const uint &nbAngularSectors )
{
	_pieChart.setNumberOfAngularSectors(nbAngularSectors);
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
