#include "inc/detection/sectorhistogram.h"

#include "inc/billon.h"
#include "inc/piechart.h"

#include <qmath.h>

SectorHistogram::SectorHistogram() : Histogram<qreal>(), _pieChart(PieChart(500))
{
}

SectorHistogram::~SectorHistogram()
{
}

/*******************************
 * Public setters
 *******************************/

void SectorHistogram::construct( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<int> &intensity,
								 const uint &zMotionMin, const int &radiusAroundPith )
{
	clear();

	if ( billon.hasPith() && sliceInterval.isValid() )
	{
		const int &width = billon.n_cols;
		const int &height = billon.n_rows;
		const qreal squareRadius = qPow(radiusAroundPith,2);

		fill(0.,_pieChart.nbSectors());

		QVector<int> circleLines;
		circleLines.reserve(2*radiusAroundPith+1);
		for ( int lineIndex=-radiusAroundPith ; lineIndex<=radiusAroundPith ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}

		QVector<int>::ConstIterator circlesLinesIterator;
		int iRadius;
		uint diff;
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
					if ( currentPos.x < width && currentPos.y < height && intensity.containsOpen(currentSlice(currentPos.y,currentPos.x)) &&
						 intensity.containsOpen(previousSlice(currentPos.y,currentPos.x)) )
					{
						diff = billon.zMotion(currentPos.x,currentPos.y,k);
						if ( diff >= zMotionMin )
						{
							const uint sectorIndex = _pieChart.sectorIndexOfAngle( currentPithCoord.angle(currentPos) );
							(*this)[sectorIndex] += (diff-zMotionMin);
//							(*this)[sectorIndex + (sectorIndex != _pieChart.nbSectors())] += (diff-zMotionMin)/3;
//							(*this)[sectorIndex==0 ? _pieChart.nbSectors()-1 : sectorIndex-1] += (diff-zMotionMin)/3;
						}
					}
					currentPos.x++;
				}
				currentPos.y++;
			}
		}
	}
}

void SectorHistogram::computeMaximumsAndIntervals( const uint &intervalGap, const bool & loop )
{
	Histogram<qreal>::computeMaximumsAndIntervals( loop );

	const uint &nbSectors = _pieChart.nbSectors();
	uint min, max;
	for ( int i=0 ; i<_intervals.size() ; ++i )
	{
		Interval<uint> &interval = _intervals[i];
		min = interval.min();
		max = interval.max();
		interval.setMin(min<intervalGap?nbSectors+min-intervalGap:min-intervalGap);
		interval.setMax(max>nbSectors-1-intervalGap?max+intervalGap-nbSectors:max+intervalGap);
	}
}

PieChart &SectorHistogram::pieChart()
{
	return _pieChart;
}

const PieChart &SectorHistogram::pieChart() const
{
	return _pieChart;
}
