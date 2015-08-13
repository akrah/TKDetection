#include "inc/detection/sectorhistogram.h"

#include "inc/billon.h"
#include "inc/piechart.h"
#include "inc/globalfunctions.h"

#include <qmath.h>

SectorHistogram::SectorHistogram() : Histogram<qreal>(), _pieChart(PieChart(500))
{
}

SectorHistogram::SectorHistogram( const PieChart &pieChart ) : Histogram<qreal>(), _pieChart(pieChart)
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
	Q_ASSERT_X( sliceInterval.isValid(), "void SectorHistogram::construct( ... )" , "L'interval de coupes n'est pas valide." );

	clear();

	if ( billon.hasPith() )
	{
		const int &width = billon.n_cols;
		const int &height = billon.n_rows;

		fill(0.,_pieChart.nbSectors());

		QVector<int> circleLines = TKD::circleLines(radiusAroundPith);

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
				currentPos.x = currentPithCoord.x-iRadius; // TODO : Est-ce que les types qreal -> int est vraiment nécessaire ?
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

const PieChart &SectorHistogram::pieChart() const
{
	return _pieChart;
}

void SectorHistogram::setPieChart( const PieChart &pieChart )
{
	_pieChart = pieChart;
}

void SectorHistogram::setSectorNumber( const uint &nbSectors )
{
	_pieChart.setNumberOfAngularSectors(nbSectors);
}

