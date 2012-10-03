#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QtGlobal>
#include "define.h"

template<typename T> class Interval;
class QImage;

class SliceView
{

public:
	explicit SliceView();

	void setTypeOfView( const SliceType &type );

	qreal flowAlpha() const;
	qreal flowEpsilon() const;
	int flowMaximumIterations() const;
	void setFlowAlpha( const qreal &alpha );
	void setFlowEpsilon( const qreal &epsilon );
	void setFlowMaximumIterations( const int &maxIter );

	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void setRestrictedAreaBeginRadius( const int &radius );

	void setEdgeDetectionType( const EdgeDetectionType &type );
	void setRadiusOfGaussianMask( const int &radius );
	void setSigmaOfGaussianMask( const qreal &sigma );
	void setCannyMinimumGradient( const qreal &minimumGradient );
	void setCannyMinimumDeviation( const qreal &minimumDeviation );

	void drawSlice( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceNumber, const Interval<int> &intensityInterval, const Interval<int> &motionInterval );

private :
	void drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval<int> &intensityInterval );
	void drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval<int> &intensityInterval, const Interval<int> &motionInterval );
	void drawEdgeDetectionSlice( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceNumber, const Interval<int> &intensityInterval );
	void drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber );
	void drawRestrictedArea( QImage &image, const Billon &billon, const iCoord2D &center, const int &sliceNumber, const int &intensityThreshold );

private:
	SliceType _typeOfView;

	qreal _flowAlpha;
	qreal _flowEpsilon;
	int _flowMaximumIterations;

	int _restrictedAreaResolution;
	int _restrictedAreaThreshold;
	int _restrictedAreaBeginRadius;

	EdgeDetectionType _typeOfEdgeDetection;
	int _cannyRadiusOfGaussianMask;
	qreal _cannySigmaOfGaussianMask;
	qreal _cannyMinimumGradient;
	qreal _cannyMinimumDeviation;
};

#endif // SLICEVIEW_H
