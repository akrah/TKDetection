#ifndef KNOTBYZMOTIONMAPDETECTOR_H
#define KNOTBYZMOTIONMAPDETECTOR_H

#include "inc/detection/knotareadetector.h"
#include "inc/detection/zmotionaccumulator.h"
#include "inc/billon.h"

#include <QMap>

class KnotByZMotionMapDetector : public KnotAreaDetector
{
public:
	KnotByZMotionMapDetector();
	virtual ~KnotByZMotionMapDetector();

	void execute( const Billon &billon );
	void clear();

	const Slice &zMotionMap() const;
	const qreal &maxFindIntensity() const;

	const __billon_type__ &binarizationThreshold() const;
	const qreal &maximumConnectedComponentDistance() const;
	const uint &minimumConnectedComponentSize() const;

	void setBinarizationThreshold( const __billon_type__ &threshold );
	void setMaximumConnectedComponentDistance( const qreal &distance );
	void setMinimumConnectedComponentSize( const uint &size );

	void setZMotionAccumulatorParameters( const Interval<int> &intensityInterval,
										  const uint &zMotionMin,
										  const uint &radiusAroundPith );

private:
	void computeKnotAreas();
	void computeLabelledImage( const Slice &accumulationSlice, Slice &labelledSlice , QMap<int, QList<iCoord2D> > &ccList );
	int findNearestConnectedComponent(const QList<iCoord2D> &currentCC, const QMap<int, QList<iCoord2D> > &supportingAreaList , qreal &minDist );

private:
	Slice _zMotionMap;
	ZMotionAccumulator _zMotionAccumulator;

	__billon_type__ _binarizationThreshold;
	qreal _maximumConnectedComponentDistance;
	uint _minimumConnectedComponentSize;
};

#endif // KNOTBYZMOTIONMAPDETECTOR_H
