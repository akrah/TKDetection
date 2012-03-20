#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include <QtGlobal>
#include "sliceview_def.h"

#include "billon_def.h"
class IntensityInterval;
class QPainter;

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

	void drawSlice( QPainter &painter, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );

private :
	void drawCurrentSlice( QPainter &painter, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );
	void drawAverageSlice( QPainter &painter, const Billon &billon, const IntensityInterval &intensityInterval );
	void drawMedianSlice( QPainter &painter, const Billon &billon, const IntensityInterval &intensityInterval );
	void drawMovementSlice( QPainter &painter, const Billon &billon, const int &sliceNumber, const IntensityInterval &intensityInterval );
	void drawFlowSlice( QPainter &painter, const Billon &billon, const int &sliceNumber );

private:
	SliceType::SliceType _typeOfView;

	int _movementThreshold;
	bool _movementWithBackground;
	bool _useNextSliceInsteadOfCurrentSlice;

	qreal _flowAlpha;
	qreal _flowEpsilon;
	int _flowMaximumIterations;
};

#endif // SLICEVIEW_H
