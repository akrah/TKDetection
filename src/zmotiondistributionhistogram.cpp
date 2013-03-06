#include "inc/zmotiondistributionhistogram.h"

#include "inc/billon.h"

ZMotionDistributionHistogram::ZMotionDistributionHistogram() : Histogram<qreal>()
{
}


ZMotionDistributionHistogram::~ZMotionDistributionHistogram()
{
}

void ZMotionDistributionHistogram::construct( const Billon &billon, const Interval<int> &intensityInterval,
											  const Interval<uint> &zMotionInterval, const uint &smoothingRadius, const int &radiusAroundPith )
{
	clear();

	if ( !billon.hasPith() ) return;

	const int &width = billon.n_cols;
	const int &height = billon.n_rows;
	const int &depth = billon.n_slices;
	const int &minVal = zMotionInterval.min();
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	int i, j, k, iRadius, iRadiusMax;
	__billon_type__ zMotion, zMotionMax;
	iCoord2D currentPos;

	resize(intensityInterval.size()+1);

	QVector<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	zMotionMax = minVal;
	for ( k=1 ; k<depth ; ++k )
	{
		const Slice &currentSlice = billon.slice(k);
		const Slice &previousSlice = billon.previousSlice(k);
		currentPos.y = billon.pithCoord(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = billon.pithCoord(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i )
			{
				if ( currentPos.x < width && currentPos.y < height
					 && intensityInterval.containsClosed(currentSlice.at(currentPos.y,currentPos.x))
					 && intensityInterval.containsClosed(previousSlice.at(currentPos.y,currentPos.x)) )
				{
					zMotion = billon.zMotion(currentPos.x,currentPos.y,k);
//					if ( zMotionInterval.containsClosed(zMotion) )
					if ( zMotion>=minVal )
					{
						zMotionMax = qMax(zMotionMax,zMotion);
						++((*this)[zMotion-minVal]);
					}
				}
				currentPos.x++;
			}
			currentPos.y++;
		}
	}

	resize(zMotionMax+1);

	meansSmoothing(smoothingRadius,false);
}
