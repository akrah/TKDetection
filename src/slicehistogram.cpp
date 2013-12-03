#include "inc/slicehistogram.h"

#include "inc/billon.h"

SliceHistogram::SliceHistogram() : Histogram<qreal>()
{
}

SliceHistogram::~SliceHistogram()
{
}

/**********************************
 * Public setters
 **********************************/

void SliceHistogram::construct( const Billon &billon, const Interval<int> &intensity, const uint &zMotionMin,
								const uint &nbSlicesToIgnore, const int &radiusAroundPith )
{
	const int width = billon.n_cols;
	const int height = billon.n_rows;
	const int depth = billon.n_slices;
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	clear();
	resize(depth-1);

	QVector<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	const uint maxOfInterval = qMin(depth-nbSlicesToIgnore,static_cast<uint>(this->size()-1));
	int i, j, iRadius, iRadiusMax;
	uint diff, k;
	iCoord2D currentPos;
	qreal cumul;

	for ( k=nbSlicesToIgnore ; k<maxOfInterval ; ++k )
	{
		cumul = 0.;
		currentPos.y = billon.pithCoord(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j, currentPos.y++ )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = billon.pithCoord(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i, currentPos.x++ )
			{
				if ( currentPos.x >= 0 && currentPos.y >= 0 && currentPos.x < width && currentPos.y < height )
				{
					if ( intensity.containsClosed(billon(currentPos.y,currentPos.x,k)) && intensity.containsClosed(billon.previousSlice(k)(currentPos.y,currentPos.x)) )
					{
						diff = billon.zMotion( currentPos.x, currentPos.y, k );
						if ( diff > zMotionMin ) cumul += diff-zMotionMin;
					}
				}
			}
		}
		(*this)[k] = cumul;
	}
}
