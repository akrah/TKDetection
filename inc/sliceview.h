#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "define.h"
#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include <QtGlobal>

template<typename T> class Interval;
class QImage;

class SliceView
{

public:
	explicit SliceView();

	void drawSlice(QImage &image, const Billon &billon, const TKD::SliceType &sliceType, const uiCoord2D &center, const uint &sliceNumber, const Interval<int> &intensityInterval,
					const Interval<int> &motionInterval, const uint &angularResokution, const TKD::ViewType &axe, const TKD::OpticalFlowParameters &opticalFlowParameters,
					const TKD::EdgeDetectionParameters &edgeDetectionParameters );

private :
	void drawCurrentSlice( QImage &image, const Billon &billon, const uint &sliceIndex,
						  const Interval<int> &intensityInterval, const uint &angularResolution, const TKD::ViewType &axe = TKD::Z_VIEW );
	void drawMovementSlice(QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
							const Interval<int> &motionInterval, const uint &angularResolution, const TKD::ViewType &axe = TKD::Z_VIEW );
	void drawEdgeDetectionSlice( QImage &image, const Billon &billon, const iCoord2D &center, const uint &sliceindex,
								 const Interval<int> &intensityInterval, const TKD::EdgeDetectionParameters &edgeDetectionParameters );
	void drawFlowSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const TKD::OpticalFlowParameters &opticalFlowParameters );
};

#endif // SLICEVIEW_H
