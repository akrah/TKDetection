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

void SliceHistogram::construct( const Billon &billon, const Interval<int> &intensity, const Interval<int> &motionInterval,
				const int & borderPercentageToCut, const int &radiusAroundPith )
{
	const int width = billon.n_cols;
	const int height = billon.n_rows;
	const int depth = billon.n_slices;
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);
	const int zMotionMin = motionInterval.min();

	clear();
	resize(depth-1);

	QVector<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	const uint minOfInterval = borderPercentageToCut*depth/100.;
	const uint maxOfInterval = qMin(depth-minOfInterval,static_cast<uint>(this->size()-1));
	int i, j, iRadius, iRadiusMax;
	__billon_type__ diff;
	iCoord2D currentPos;
	uint k;
	qreal cumul;

	for ( k=minOfInterval ; k<maxOfInterval ; ++k )
	{
		cumul = 0.;
		currentPos.y = billon.pithCoord(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = billon.pithCoord(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i )
			{
				if ( currentPos.x >= 0 && currentPos.y >= 0 && currentPos.x < width && currentPos.y < height )
				{
					if ( intensity.containsClosed(billon.slice(k).at(currentPos.y,currentPos.x)) && intensity.containsClosed(billon.previousSlice(k).at(currentPos.y,currentPos.x)) )
					{
						diff = billon.zMotion( currentPos.x, currentPos.y, k );
						if ( diff > zMotionMin ) cumul += diff-zMotionMin;
					}
				}
				currentPos.x++;
			}
			currentPos.y++;
		}
		(*this)[k] = cumul;
	}
}
