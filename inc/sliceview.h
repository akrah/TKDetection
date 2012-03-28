#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QtGlobal>
#include "sliceview_def.h"

#include "billon_def.h"
class IntensityInterval;
class QImage;

class SliceView
{

public:
	explicit SliceView();

	void setTypeOfView( const SliceType::SliceType &type );

	void setMovementThreshold( const int &threshold );
	void enableMovementWithBackground( const bool &enable );
	void useNextSliceInsteadOfCurrentSlice( const bool &enable );

	qreal flowAlpha() const;
	qreal flowEpsilon() const;
	int flowMaximumIterations() const;
	void setFlowAlpha( const qreal &alpha );
	void setFlowEpsilon( const qreal &epsilon );
	void setFlowMaximumIterations( const int &maxIter );

	void setRestrictedAreaResolution( const int &resolution );
	void setRestrictedAreaThreshold( const int &threshold );
	void enableRestrictedAreaCircle( const bool &enable );

	void drawSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );

private :
	void drawCurrentSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );
	void drawAverageSlice( QImage &image, const Billon &billon, const IntensityInterval &intensityInterval );
	void drawMedianSlice( QImage &image, const Billon &billon, const IntensityInterval &intensityInterval );
	void drawMovementSlice( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );
	void drawFlowSlice( QImage &image, const Billon &billon, const int &sliceNumber );
	void drawRestrictedArea( QImage &image, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );

private:
	SliceType::SliceType _typeOfView;

	int _movementThreshold;
	bool _movementWithBackground;
	bool _useNextSliceInsteadOfCurrentSlice;

	qreal _flowAlpha;
	qreal _flowEpsilon;
	int _flowMaximumIterations;

	int _restrictedAreaResolution;
	int _restrictedAreaThreshold;
	bool _restrictedAreaDrawCircle;
};

#endif // SLICEVIEW_H
