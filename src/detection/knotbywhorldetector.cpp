#include "inc/detection/knotbywhorldetector.h"

#include "inc/billon.h"

KnotByWhorlDetector::KnotByWhorlDetector() : KnotAreaDetector(), _intensityInterval(MINIMUM_INTENSITY,MAXIMUM_INTENSITY), _zMotionMin(MINIMUM_Z_MOTION)
{}

KnotByWhorlDetector::~KnotByWhorlDetector() {}

void KnotByWhorlDetector::execute( const Billon &billon )
{
	clear();

	if ( !billon.hasPith() ) return;

//	// Compute the z-motion slice histogram
//	_sliceHistogram.construct(billon, _intensityInterval,
//							   _ui->_spinZMotionMin->value(), _ui->_spinHistogramPercentageOfSlicesToIgnore_zMotion->value()*_billon->n_slices/100., _treeRadius*_ui->_spinRestrictedAreaPercentage->value()/100.);
//	// Detect slice intervals of knot whorls
//	_sliceHistogram->computeMaximumsAndIntervals( _ui->_spinHistogramSmoothingRadius_zMotion->value(),
//												  _ui->_spinHistogramMinimumHeightOfMaximum_zMotion->value(),
//												  _ui->_spinHistogramDerivativeSearchPercentage_zMotion->value(),
//												  _ui->_spinHistogramMinimumWidthOfInterval_zMotion->value(), false);
}

void KnotByWhorlDetector::clear()
{
	KnotAreaDetector::clear();
	_sliceHistogram.clear();
	_sectorHistograms.clear();
}

const SliceHistogram &KnotByWhorlDetector::sliceHistogram() const
{
	return _sliceHistogram;
}

const SectorHistogram &KnotByWhorlDetector::sectorHistogram( const uint &whorlId ) const
{
	return _sectorHistograms[whorlId];
}

const Interval<int> &KnotByWhorlDetector::intensityInterval() const
{
	return _intensityInterval;
}

void KnotByWhorlDetector::setIntensityInterval( const Interval<int> interval )
{
	_intensityInterval = interval;
}

const uint &KnotByWhorlDetector::zMotionMin() const
{
	return _zMotionMin;
}

void KnotByWhorlDetector::setZMotionMin( const uint &value )
{
	_zMotionMin = value;
}
