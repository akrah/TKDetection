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
	const SectorHistogram &sectorHistogram( const uint &whorlId ) const;

private:
	SliceHistogram _sliceHistogram;
	QVector<SectorHistogram> _sectorHistograms;
};

#endif // KNOTBYWHORLDETECTOR_H
