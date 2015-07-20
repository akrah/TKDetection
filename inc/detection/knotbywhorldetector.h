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

	void updateSliceHistogram( const Billon &billon );
	void updateSectorHistograms( const Billon &billon );
	void updateSectorHistogram( const Billon &billon, const uint &whorlIndex );

	const SliceHistogram &sliceHistogram() const;
	const SectorHistogram &sectorHistogram( const uint &whorlIndex ) const;

	void setSliceHistogramParameters( const uint &smoothingRadius,
									  const uint &minimumHeightOfMaximums,
									  const uint &derivativeSearchPercentage,
									  const uint &minimumWidthOfInterval );

	void setSectorHistogramsParameters( const uint &smoothingRadius,
										const uint &minimumHeightOfMaximums,
										const uint &derivativeSearchPercentage,
										const uint &minimumWidthOfInterval );

private:
	SliceHistogram _sliceHistogram;
	QVector<SectorHistogram> _sectorHistograms;

	uint _sectorHist_smoothingRadius;
	uint _sectorHist_minimumHeightOfMaximums;
	uint _sectorHist_derivativeSearchPercentage;
	uint _sectorHist_minimumWidthOfInterval;
};

#endif // KNOTBYWHORLDETECTOR_H
