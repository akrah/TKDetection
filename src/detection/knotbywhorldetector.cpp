#include "inc/detection/knotbywhorldetector.h"

#include "inc/billon.h"
#include "inc/define.h"

KnotByWhorlDetector::KnotByWhorlDetector() : KnotAreaDetector(),
	_sectorHist_smoothingRadius(HISTOGRAM_ANGULAR_SMOOTHING_RADIUS),
	_sectorHist_minimumHeightOfMaximums(HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM),
	_sectorHist_derivativeSearchPercentage(HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE),
	_sectorHist_minimumWidthOfInterval(HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS)
{}

KnotByWhorlDetector::~KnotByWhorlDetector() {}

void KnotByWhorlDetector::execute( const Billon &billon )
{
	updateSliceHistogram( billon );
	updateSectorHistograms( billon );
}

void KnotByWhorlDetector::clear()
{
	KnotAreaDetector::clear();
	_sliceHistogram.clear();
	_sectorHistograms.clear();
}

void KnotByWhorlDetector::updateSliceHistogram( const Billon &billon )
{
	clear();

	if ( !billon.hasPith() ) return;

	// Compute the z-motion slice histogram
	_sliceHistogram.construct( billon, _intensityInterval, _zMotionMin, _treeRadius );
	// Detect slice intervals of knot whorls
	_sliceHistogram.computeMaximumsAndIntervals(false);
}

void KnotByWhorlDetector::updateSectorHistograms( const Billon &billon )
{
	if ( ! _sliceHistogram.nbIntervals() ) return;

	const uint nbWhorls = _sliceHistogram.nbIntervals();
	_sectorHistograms.resize(nbWhorls);

	for ( uint k=0 ; k<nbWhorls ; ++k )
	{
		SectorHistogram &sectorHistogram = _sectorHistograms[k];
		sectorHistogram.setPieChart(_pieChart);

		// Compute the sector histogram of each detected whorl
		sectorHistogram.construct( billon, _sliceHistogram.interval(k), _intensityInterval, _zMotionMin, _treeRadius );

		// Set all parameters for the intervalle detection
		sectorHistogram.setSmoothingRadius(_sectorHist_smoothingRadius);
		sectorHistogram.setMinimumHeightPercentageOfMaximum(_sectorHist_minimumHeightOfMaximums);
		sectorHistogram.setDerivativesPercentage(_sectorHist_derivativeSearchPercentage);
		sectorHistogram.setMinimumIntervalWidth(_sectorHist_minimumWidthOfInterval);

		// Compute angular intervals of knots for this whorl
		sectorHistogram.computeMaximumsAndIntervals(true);
	}
}

void KnotByWhorlDetector::updateSectorHistogram( const Billon &billon, const uint &whorlIndex )
{
	if ( _sliceHistogram.nbIntervals() >= whorlIndex ) return;

	SectorHistogram &sectorHistogram = _sectorHistograms[whorlIndex];
	sectorHistogram.setPieChart(_pieChart);

	// Compute the sector histogram of each detected whorl
	sectorHistogram.construct( billon, _sliceHistogram.interval(whorlIndex), _intensityInterval, _zMotionMin, _treeRadius );

	// Set all parameters for the intervalle detection
	sectorHistogram.setSmoothingRadius(_sectorHist_smoothingRadius);
	sectorHistogram.setMinimumHeightPercentageOfMaximum(_sectorHist_minimumHeightOfMaximums);
	sectorHistogram.setDerivativesPercentage(_sectorHist_derivativeSearchPercentage);
	sectorHistogram.setMinimumIntervalWidth(_sectorHist_minimumWidthOfInterval);

	// Compute angular intervals of knots for this whorl
	sectorHistogram.computeMaximumsAndIntervals(true);
}

const SliceHistogram &KnotByWhorlDetector::sliceHistogram() const
{
	return _sliceHistogram;
}

const SectorHistogram &KnotByWhorlDetector::sectorHistogram( const uint &whorlIndex ) const
{
	Q_ASSERT_X( whorlIndex<static_cast<uint>(_sectorHistograms.size()) , "const SectorHistogram & sectorHistogram( const uint &whorlIndex )" , "whorlIndex doit être inférieur au nombre de verticilles" );
	return _sectorHistograms[whorlIndex];
}

void KnotByWhorlDetector::setSliceHistogramParameters( const uint &smoothingRadius,
								  const uint &minimumHeightOfMaximums,
								  const uint &derivativeSearchPercentage,
								  const uint &minimumWidthOfInterval )
{
	_sliceHistogram.setSmoothingRadius(smoothingRadius);
	_sliceHistogram.setMinimumHeightPercentageOfMaximum(minimumHeightOfMaximums);
	_sliceHistogram.setDerivativesPercentage(derivativeSearchPercentage);
	_sliceHistogram.setMinimumIntervalWidth(minimumWidthOfInterval);
}

void KnotByWhorlDetector::setSectorHistogramsParameters( const uint &smoothingRadius,
								  const uint &minimumHeightOfMaximums,
								  const uint &derivativeSearchPercentage,
								  const uint &minimumWidthOfInterval )
{
	_sectorHist_smoothingRadius = smoothingRadius;
	_sectorHist_minimumHeightOfMaximums = minimumHeightOfMaximums;
	_sectorHist_derivativeSearchPercentage = derivativeSearchPercentage;
	_sectorHist_minimumWidthOfInterval = minimumWidthOfInterval;
}
