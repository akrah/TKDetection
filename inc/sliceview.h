#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QtGlobal>
#include "sliceview_def.h"

#include "billon_def.h"
#include "interval.h"
class QImage;
class Marrow;

class SliceView
{

public:
	explicit SliceView();

	void setTypeOfView( const SliceType::SliceType &type );

	void useNextSliceInsteadOfCurrentSlice( const bool &enable );

	qreal flowAlpha() const;
	qreal flowEpsilon() const;
	int flowMaximumIterations() const;
	void setFlowAlpha( const qreal &alpha );
	void setFlowEpsilon( const qreal &epsilon );
	void setFlowMaximumIterations( const int &maxIter );

	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void setRestrictedAreaBeginRadius( const int &radius );

	void setEdgeDetectionType( const EdgeDetectionType::EdgeDetectionType &type );
	void setRadiusOfGaussianMask( const int &radius );
	void setSigmaOfGaussianMask( const qreal &sigma );
	void setCannyMinimumGradient( const qreal &minimumGradient );
	void setCannyMinimumDeviation( const qreal &minimumDeviation );

	void drawSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval, const Interval &motionInterval );

private :
	void drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval );
	void drawAverageSlice( QImage &image, const Billon &billon, const Interval &intensityInterval );
	void drawMedianSlice( QImage &image, const Billon &billon, const Interval &intensityInterval );
	void drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const Interval &intensityInterval, const Interval &motionInterval );
	void drawEdgeDetectionSlice( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval );
	void drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber );
	void drawRestrictedArea( QImage &image, const Billon &billon, const Marrow *marrow, const int &sliceNumber, const Interval &intensityInterval );

private:
	SliceType::SliceType _typeOfView;
	bool _useNextSliceInsteadOfCurrentSlice;

	qreal _flowAlpha;
	qreal _flowEpsilon;
	int _flowMaximumIterations;

	int _restrictedAreaResolution;
	int _restrictedAreaThreshold;
	int _restrictedAreaBeginRadius;

	EdgeDetectionType::EdgeDetectionType _typeOfEdgeDetection;
	int _cannyRadiusOfGaussianMask;
	qreal _cannySigmaOfGaussianMask;
	qreal _cannyMinimumGradient;
	qreal _cannyMinimumDeviation;
};

#endif // SLICEVIEW_H
