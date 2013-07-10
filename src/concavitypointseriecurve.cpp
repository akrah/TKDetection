#include "inc/concavitypointseriecurve.h"

#include "inc/contourbillon.h"
#include "inc/contourslice.h"

ConcavityPointSerieCurve::ConcavityPointSerieCurve()
{
}

ConcavityPointSerieCurve::~ConcavityPointSerieCurve()
{
}

const QVector<rCoord2D> &ConcavityPointSerieCurve::leftConcavityPointsSerie() const
{
	return _leftConcavityPointsSerie;
}

const QVector<rCoord2D> &ConcavityPointSerieCurve::rightConcavityPointsSerie() const
{
	return _rightConcavityPointsSerie;
}

int ConcavityPointSerieCurve::nbLeftConcavityPoints() const
{
	return _leftConcavityPointsSerie.size();
}

int ConcavityPointSerieCurve::nbRightConcavityPoints() const
{
	return _rightConcavityPointsSerie.size();
}

void ConcavityPointSerieCurve::clear()
{
	_leftConcavityPointsSerie.clear();
	_rightConcavityPointsSerie.clear();
}

void ConcavityPointSerieCurve::construct( const ContourBillon &contourBillon )
{
	const QVector<ContourSlice> &contourSlices = contourBillon.contourSlices();
	const int nbSlices = contourSlices.size();

	_leftConcavityPointsSerie.reserve(nbSlices);
	_rightConcavityPointsSerie.reserve(nbSlices);

	for ( int i=0 ; i<nbSlices ; ++i )
	{
		const iCoord2D &leftConcavityPoint = contourSlices[i].leftConcavityPoint();
		if ( leftConcavityPoint != invalidICoord2D )
		{
			_leftConcavityPointsSerie.append(rCoord2D(i,leftConcavityPoint.angle(contourSlices[i].leftSupportPoint())));
		}
		const iCoord2D &rightConcavityPoint = contourSlices[i].rightConcavityPoint();
		if ( rightConcavityPoint != invalidICoord2D )
		{
			_rightConcavityPointsSerie.append(rCoord2D(i,rightConcavityPoint.angle(contourSlices[i].rightSupportPoint())));
		}
	}
}
