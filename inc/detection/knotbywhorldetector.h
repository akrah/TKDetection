#ifndef KNOTBYWHORLDETECTOR_H
#define KNOTBYWHORLDETECTOR_H

#include "knotareadetector.h"

#include "slicehistogram.h"
#include "sectorhistogram.h"
#include "inc/interval.h"

#include <QVector>

class KnotByWhorlDetector : public KnotAreaDetector
{
public:
	KnotByWhorlDetector();
	virtual ~KnotByWhorlDetector();

	void execute( const Billon &billon );
	void clear();

	const SliceHistogram &sliceHistogram() const;
	const SectorHistogram &sectorHistogram( const uint &whorlId ) const;

	const Interval<int> &intensityInterval() const;
	void setIntensityInterval( const Interval<int> interval );

	const uint &zMotionMin() const;
	void setZMotionMin( const uint &value );

private:
	SliceHistogram _sliceHistogram;
	QVector<SectorHistogram> _sectorHistograms;
	Interval<int> _intensityInterval;
	uint _zMotionMin;
};

#endif // KNOTBYWHORLDETECTOR_H
