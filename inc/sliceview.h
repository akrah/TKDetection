#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "sliceview_def.h"

class Billon;
class Marrow;
class QPainter;

class SliceView
{

public:
	explicit SliceView();

	void setModel( const Billon * billon );
	void setModel( const Marrow * marrow );

	void setTypeOfView( const SliceType::SliceType &type );

	void drawSlice( QPainter &painter, const int &sliceNumber );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void setMotionThreshold( const int &threshold );

private :
	void drawCurrentSlice( QPainter &painter, const int &sliceNumber );
	void drawAverageSlice( QPainter &painter );
	void drawMedianSlice( QPainter &painter );
	void drawMovementSlice( QPainter &painter, const int &sliceNumber );

private:
	const Billon * _billon;
	const Marrow * _marrow;

	int _lowThreshold;
	int _highThreshold;

	SliceType::SliceType _typeOfView;

	int _motionThreshold;
};

#endif // SLICEVIEW_H
