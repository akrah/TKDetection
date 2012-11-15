#include "inc/contourdistanceshistogram.h"

#include "inc/contour.h"
#include "inc/coordinate.h"

ContourDistancesHistogram::ContourDistancesHistogram()
{
}

ContourDistancesHistogram::~ContourDistancesHistogram()
{
}

void ContourDistancesHistogram::construct( const Contour &contour, const iCoord2D &pithCoord )
{
	clear();

	const int nbPoints = contour.size();
	if ( nbPoints > 0 )
	{
		this->resize(nbPoints);

		QVector<qreal>::Iterator thisIterator = this->begin();
		QVector<iCoord2D>::ConstIterator contourIterator = contour.begin();
		const QVector<iCoord2D>::ConstIterator endContour = contour.end();
		while ( contourIterator != endContour )
		{
			*thisIterator++ = (*contourIterator++).euclideanDistance(pithCoord);
		}
	}
}
