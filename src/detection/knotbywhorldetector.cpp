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
	computeKnotAreas();
}

void KnotByWhorlDetector::computeKnotAreas()
{
	KnotAreaDetector::clear();
	if ( _sliceHistogram.isEmpty() || _sectorHistograms.isEmpty() || _sliceHistogram.nbIntervals() != static_cast<uint>(_sectorHistograms.size()) ) return;

	uint k, i, maxValIndex, angularWidth;
	for ( k=0 ; k<_sliceHistogram.nbIntervals() ; ++k )
	{
		const Interval<uint> &sliceInterval = _sliceHistogram.interval(k);
		const SectorHistogram &sectorHistogram = _sectorHistograms[k];
		for ( i=0 ; i<sectorHistogram.nbIntervals() ; ++i )
		{
			qDebug() << "Interval " << i << " : [" << sectorHistogram.interval(i).min() << ", " << sectorHistogram.interval(i).max() << "]";
			const Interval<uint> &sectorInterval = sectorHistogram.interval(i);
			maxValIndex = sectorHistogram.maximumIndex(i);
			angularWidth = sectorInterval.width();
			_knotAreas.append( KnotArea( sliceInterval.min(), maxValIndex>angularWidth/2 ? maxValIndex-angularWidth/2 : maxValIndex+_pieChart.nbSectors()-angularWidth/2, sliceInterval.width(), angularWidth ) );
		}
	}
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

const SectorHistogram &KnotByWhorlDetector::sectorHistogram( const uint &whorlIndex ) const
{
	Q_ASSERT_X( whorlIndex<static_cast<uint>(_sectorHistograms.size()) , "const SectorHistogram & sectorHistogram( const uint &whorlIndex )" , "whorlIndex doit être inférieur au nombre de verticilles" );
	return _sectorHistograms[whorlIndex];
}

bool KnotByWhorlDetector::hasSectorHistograms() const
{
	return !_sectorHistograms.isEmpty();
}

uint KnotByWhorlDetector::knotAreaIndex( const uint &whorlIndex, const uint &angularIntervalIndex ) const
{
	Q_ASSERT_X( whorlIndex<_sliceHistogram.nbIntervals() && angularIntervalIndex<_sectorHistograms[whorlIndex].nbIntervals() ,
				"const QVector<QRect> &knotAreas( const uint &whorlIndex, const uint &angularIntervalIndex )",
				"whorlIndex doit être inférieur au nombre de verticilles et angularIntervalIndex doit être inférieur au nombre d'intervalles angulaire de ce vergicille.");
	uint knotAreaIndex = angularIntervalIndex;
	for ( uint i=0 ; i<whorlIndex ; ++i )
	{
		knotAreaIndex += _sectorHistograms[i].nbIntervals();
	}
	return knotAreaIndex;
}

const KnotArea &KnotByWhorlDetector::knotArea( const uint &whorlIndex, const uint &angularIntervalIndex ) const
{
	return _knotAreas[knotAreaIndex(whorlIndex,angularIntervalIndex)];
}

const Interval<uint> KnotByWhorlDetector::centeredSectorInterval( const uint &whorlIndex, const uint &angularIntervalIndex ) const
{
	return knotArea( whorlIndex, angularIntervalIndex ).sectorInterval();
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
	_sectorHistograms.clear();

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
