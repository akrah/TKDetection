#ifndef KNOTBYWHORLDETECTOR_H
#define KNOTBYWHORLDETECTOR_H

#include "knotareadetector.h"

#include "slicehistogram.h"
#include "sectorhistogram.h"

class KnotByWhorlDetector : public KnotAreaDetector
{
public:
	KnotByWhorlDetector();
	virtual ~KnotByWhorlDetector();

	void execute( const Billon &billon );
	void clear();

	const SliceHistogram &sliceHistogram() const;
	const SectorHistogram &sectorHistogram( const uint &whorlIndex ) const;
	bool hasSectorHistograms() const;

	using KnotAreaDetector::knotArea;
	uint knotAreaIndex( const uint &whorlIndex, const uint &angularIntervalIndex ) const;
	const KnotArea &knotArea( const uint &whorlIndex, const uint &angularIntervalIndex ) const;
	const Interval<uint> centeredSectorInterval( const uint &whorlIndex, const uint &angularIntervalIndex ) const;

	void setSliceHistogramParameters( const uint &smoothingRadius,
									  const uint &minimumHeightOfMaximums,
									  const uint &derivativeSearchPercentage,
									  const uint &minimumWidthOfInterval );

	void setSectorHistogramsParameters( const uint &smoothingRadius,
										const uint &minimumHeightOfMaximums,
										const uint &derivativeSearchPercentage,
										const uint &minimumWidthOfInterval );

private:
	void computeKnotAreas();
	void updateSliceHistogram( const Billon &billon );
	void updateSectorHistograms( const Billon &billon );
	void updateSectorHistogram( const Billon &billon, const uint &whorlIndex );

private:
	SliceHistogram _sliceHistogram;
	QVector<SectorHistogram> _sectorHistograms;

	uint _sectorHist_smoothingRadius;
	uint _sectorHist_minimumHeightOfMaximums;
	uint _sectorHist_derivativeSearchPercentage;
	uint _sectorHist_minimumWidthOfInterval;
};

#endif // KNOTBYWHORLDETECTOR_H
