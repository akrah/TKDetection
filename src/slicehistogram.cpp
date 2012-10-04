#include "inc/slicehistogram.h"

#include "inc/billon.h"

SliceHistogram::SliceHistogram()
{
}

SliceHistogram::~SliceHistogram()
{
}

/**********************************
 * Public setters
 **********************************/

void SliceHistogram::construct( const Billon &billon, const Marrow &marrow, const Interval<int> &intensity, const Interval<int> &motionInterval,
				const int & borderPercentageToCut, const int &radiusAroundPith )
{
	const int width = billon.n_cols;
	const int height = billon.n_rows;
	const int depth = billon.n_slices;
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	this->clear();
	this->resize(depth-1);

	QVector<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	int i, j, k, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	iCoord2D currentPos;
	qreal cumul, diff;

	const int kLimitMin = borderPercentageToCut*depth/100.;
	const int kLimitMax = depth-kLimitMin;
	for ( k=kLimitMin ; k<kLimitMax ; ++k )
	{
		const Slice &currentSlice = billon.slice(k);
		const Slice &previousSlice = billon.previousSlice(k);
		cumul = 0.;
		currentPos.y = marrow.at(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = marrow.at(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i )
			{
				if ( currentPos.x < width && currentPos.y < height )
				{
					currentSliceValue = currentSlice.at(currentPos.y,currentPos.x);
					previousSliceValue = previousSlice.at(currentPos.y,currentPos.x);
					if ( intensity.containsClosed(currentSliceValue) && intensity.containsClosed(previousSliceValue) )
					{
						diff = qAbs(currentSliceValue - previousSliceValue);
						if ( motionInterval.containsClosed(diff) ) cumul += diff;
					}
				}
				currentPos.x++;
			}
			currentPos.y++;
		}
		(*this)[k] = cumul;
	}
}
