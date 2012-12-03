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

	void setTypeOfView( const TKD::SliceType &type );

	qreal flowAlpha() const;
	qreal flowEpsilon() const;
	int flowMaximumIterations() const;
	void setFlowAlpha( const qreal &alpha );
	void setFlowEpsilon( const qreal &epsilon );
	void setFlowMaximumIterations( const int &maxIter );

	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void setRestrictedAreaBeginRadius( const int &radius );

	void setEdgeDetectionType( const TKD::EdgeDetectionType &type );
	void setRadiusOfGaussianMask( const int &radius );
	void setSigmaOfGaussianMask( const qreal &sigma );
	void setCannyMinimumGradient( const qreal &minimumGradient );
	void setCannyMinimumDeviation( const qreal &minimumDeviation );

	void drawSlice( QImage &image, const Billon &billon, const iCoord2D &center, const uint &sliceNumber, const Interval<int> &intensityInterval,
					const Interval<int> &motionInterval, const TKD::ViewType &axe = TKD::Z_VIEW );

private :
	void drawCurrentSlice(QImage &image, const Billon &billon, const uint &sliceIndex,
						  const Interval<int> &intensityInterval , const TKD::ViewType &axe = TKD::Z_VIEW );
	void drawMovementSlice( QImage &image, const Billon &billon, const uint &sliceIndex, const Interval<int> &intensityInterval,
							const Interval<int> &motionInterval, const TKD::ViewType &axe = TKD::Z_VIEW );
	void drawEdgeDetectionSlice( QImage &image, const Billon &billon, const iCoord2D &center, const uint &sliceindex, const Interval<int> &intensityInterval );
	void drawFlowSlice( QImage &image, const Billon &billon, const uint &sliceIndex );
	void drawRestrictedArea( QImage &image, const Billon &billon, const iCoord2D &center, const uint &sliceIndex, const int &intensityThreshold );

private:
	TKD::SliceType _sliceType;

	qreal _flowAlpha;
	qreal _flowEpsilon;
	int _flowMaximumIterations;

	int _restrictedAreaResolution;
	int _restrictedAreaThreshold;
	int _restrictedAreaBeginRadius;

	TKD::EdgeDetectionType _typeOfEdgeDetection;
	int _cannyRadiusOfGaussianMask;
	qreal _cannySigmaOfGaussianMask;
	qreal _cannyMinimumGradient;
	qreal _cannyMinimumDeviation;
};

#endif // SLICEVIEW_H
