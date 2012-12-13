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

void SectorHistogram::construct(const Billon &billon, const PieChart &pieChart, const Interval<uint> &sliceInterval, const Interval<int> &intensity,
								 const Interval<int> &motionInterval, const int &radiusAroundPith , const uint &intervalGap )
{
	clear();
	_intervalGap = intervalGap;

	if ( billon.hasPith() && sliceInterval.isValid() && sliceInterval.width() > 0 )
	{
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const qreal squareRadius = qPow(radiusAroundPith,2);

		fill(0.,pieChart.nbSectors());

		QList<int> circleLines;
		circleLines.reserve(2*radiusAroundPith+1);
		for ( int lineIndex=-radiusAroundPith ; lineIndex<=radiusAroundPith ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}

		QList<int>::ConstIterator circlesLinesIterator;
		int iRadius;
		__billon_type__ diff;
		iCoord2D currentPos;

		// Calcul du diagramme en parcourant les tranches du billon comprises dans l'intervalle
		for ( uint k=sliceInterval.min() ; k<=sliceInterval.max() ; ++k )
		{
			const Slice &currentSlice = billon.slice(k);
			const Slice &previousSlice = billon.previousSlice(k);
			const iCoord2D &currentPithCoord = billon.pithCoord(k);
			currentPos.y = currentPithCoord.y-radiusAroundPith;
			for ( circlesLinesIterator = circleLines.constBegin() ; circlesLinesIterator != circleLines.constEnd() ; ++circlesLinesIterator )
			{
				iRadius = *circlesLinesIterator;
				currentPos.x = currentPithCoord.x-iRadius;
				iRadius += currentPithCoord.x;
				while ( currentPos.x <= iRadius )
				{
					if ( currentPos.x < width && currentPos.y < height && intensity.containsOpen(currentSlice.at(currentPos.y,currentPos.x)) &&
						 intensity.containsOpen(previousSlice.at(currentPos.y,currentPos.x)) )
					{
						diff = billon.zMotion(currentPos.y,currentPos.x,k);
						if ( motionInterval.containsClosed(diff) )
						{
							(*this)[pieChart.sectorIndexOfAngle( currentPithCoord.angle(currentPos) )] += (diff-motionInterval.min());
						}
					}
					currentPos.x++;
				}
				currentPos.y++;
			}
		}
	}
}

void SectorHistogram::computeIntervals( const int &derivativesPercentage, const uint &minimumWidthOfIntervals, const bool &loop )
{
	Histogram<qreal>::computeIntervals( derivativesPercentage, minimumWidthOfIntervals, loop );
	uint min, max;
	for ( int i=0 ; i<_intervals.size() ; ++i )
	{
		Interval<uint> &interval = _intervals[i];
		min = interval.min();
		max = interval.max();
		interval.setMin(min<_intervalGap?359+min-_intervalGap:min-_intervalGap);
		// TODO : Faire de PieChart un singleton
		interval.setMax(max>354?max+_intervalGap-359:max+_intervalGap);
	}
}
