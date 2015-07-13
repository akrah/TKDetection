#include "inc/detection/knotbywhorldetector.h"

#include "inc/billon.h"

KnotByWhorlDetector::KnotByWhorlDetector() : KnotAreaDetector()
{}

KnotByWhorlDetector::~KnotByWhorlDetector() {}

void KnotByWhorlDetector::execute( const Billon &billon )
{
	clear();

	if ( !billon.hasPith() ) return;

	// Compute the z-motion slice histogram
	_sliceHistogram.construct( billon, _intensityInterval, _zMotionMin, _treeRadius );
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

