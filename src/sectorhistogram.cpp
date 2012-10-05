#include "inc/sectorhistogram.h"

#include "inc/billon.h"
#include "inc/piechart.h"

#include <qmath.h>

SectorHistogram::SectorHistogram() : Histogram<qreal>()
{
}

SectorHistogram::~SectorHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void SectorHistogram::construct( const Billon &billon, const Marrow &marrow, const PieChart &pieChart, const Interval<int> &slicesInterval, const Interval<int> &intensity,
								 const Interval<int> &motionInterval, const int &radiusAroundPith )
{
	clear();
	if ( slicesInterval.isValid() && slicesInterval.width() > 0 )
	{
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int nbSectors = pieChart.nbSectors();
		const int radiusMax = radiusAroundPith+1;
		const qreal squareRadius = qPow(radiusAroundPith,2);

		fill(0.,nbSectors);

		QList<int> circleLines;
		circleLines.reserve(2*radiusAroundPith+1);
		for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}

		const uint minOfInterval = slicesInterval.min();
		const uint maxOfInterval = slicesInterval.max();
		int i, j, iRadius, iRadiusMax;
		__billon_type__ currentSliceValue, previousSliceValue, diff;
		iCoord2D currentPos;
		uint k;

		// Calcul du diagramme en parcourant les tranches du billon comprises dans l'intervalle
		for ( k=minOfInterval ; k<maxOfInterval ; ++k )
		{
			const Slice &currentSlice = billon.slice(k);
			const Slice &previousSlice = billon.previousSlice(k);
			const iCoord2D &currentMarrowCoord = marrow[k];
			currentPos.y = currentMarrowCoord.y-radiusAroundPith;
			for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
			{
				iRadius = circleLines[j+radiusAroundPith];
				iRadiusMax = iRadius+1;
				currentPos.x = currentMarrowCoord.x-iRadius;
				for ( i=-iRadius ; i<iRadiusMax ; ++i )
				{
					if ( currentPos.x < width && currentPos.y < height )
					{
						currentSliceValue = currentSlice.at(currentPos.y,currentPos.x);
						previousSliceValue = previousSlice.at(currentPos.y,currentPos.x);
						if ( intensity.containsOpen(currentSliceValue) && intensity.containsOpen(previousSliceValue) )
						{
							diff = qAbs(currentSliceValue - previousSliceValue);
							if ( motionInterval.containsClosed(diff) )
							{
								(*this)[pieChart.sectorIndexOfAngle( currentMarrowCoord.angle(currentPos) )] += (diff-motionInterval.min());
							}
						}
					}
					currentPos.x++;
				}
				currentPos.y++;
			}
		}
	}
}
