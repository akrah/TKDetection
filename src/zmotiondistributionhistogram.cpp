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
	const int &width = billon.n_cols;
	const int &height = billon.n_rows;
	const int &depth = billon.n_slices;
	const int &minVal = zMotionInterval.min();
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	int i, j, k, iRadius, iRadiusMax;
	__billon_type__ zMotion;
	iCoord2D currentPos;

	clear();
	resize(zMotionInterval.size()+1);

	QList<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	for ( k=1 ; k<depth ; ++k )
	{
		currentPos.y = billon.pithCoord(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = billon.pithCoord(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i )
			{
				if ( currentPos.x < width && currentPos.y < height )
				{
					zMotion = billon.zMotion(currentPos.y,currentPos.x,k);
					if ( intensityInterval.containsClosed(billon.at(currentPos.y,currentPos.x,k))
						 && intensityInterval.containsClosed(billon.at(currentPos.y,currentPos.x,k-1))
						 && zMotionInterval.containsClosed(zMotion)
						 )
						++((*this)[zMotion-minVal]);
				}
				currentPos.x++;
			}
			currentPos.y++;
		}
	}

	meansSmoothing(smoothingRadius,false);
}
