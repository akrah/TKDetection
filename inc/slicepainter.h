#ifndef SLICEPAINTER_H
#define SLICEPAINTER_H

#include "define.h"
#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include <QtGlobal>

template<typename T> class Interval;
class QImage;

class SlicePainter
{

public:
	explicit SlicePainter();

	void drawSlice(QImage &image, const Billon &billon, const TKD::ViewType &sliceType, const uint &sliceNumber, const Interval<int> &intensityInterval,
			   const uint &zMotionMin, const uint &angularResokution, const TKD::ProjectionType &axe, const qreal &ellipticityRate = 1. );

private :
	void drawCurrentSlice(QImage &image, const Billon &billon, const uint &sliceIndex,
						  const Interval<int> &intensityInterval, const uint &angularResolution, const TKD::ProjectionType &axe = TKD::Z_PROJECTION,
						  const qreal &ellipticityRate = 1. );
	void drawMovementSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
							const uint &zMotionMin, const uint &angularResolution, const TKD::ProjectionType &axe = TKD::Z_PROJECTION );
	void drawHoughSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const TKD::ProjectionType &axe = TKD::Z_PROJECTION );
};

#endif // SLICEPAINTER_H
