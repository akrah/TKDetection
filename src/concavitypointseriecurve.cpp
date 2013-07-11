#include "inc/concavitypointseriecurve.h"

#include "inc/contourbillon.h"
#include "inc/contourslice.h"

ConcavityPointSerieCurve::ConcavityPointSerieCurve()
{
}

ConcavityPointSerieCurve::~ConcavityPointSerieCurve()
{
}

const QVector<rCoord2D> &ConcavityPointSerieCurve::maxConcavityPointsSerie() const
{
	return _maxConcavityPointsSerie;
}

const QVector<rCoord2D> &ConcavityPointSerieCurve::minConcavityPointsSerie() const
{
	return _minConcavityPointsSerie;
}

int ConcavityPointSerieCurve::nbMaxConcavityPoints() const
{
	return _maxConcavityPointsSerie.size();
}

int ConcavityPointSerieCurve::nbMinConcavityPoints() const
{
	return _minConcavityPointsSerie.size();
}

void ConcavityPointSerieCurve::clear()
{
	_maxConcavityPointsSerie.clear();
	_minConcavityPointsSerie.clear();
}

void ConcavityPointSerieCurve::construct( const ContourBillon &contourBillon )
{
	const QVector<ContourSlice> &contourSlices = contourBillon.contourSlices();
	const int nbSlices = contourSlices.size();

	_maxConcavityPointsSerie.reserve(nbSlices);
	_minConcavityPointsSerie.reserve(nbSlices);

	for ( int i=0 ; i<nbSlices ; ++i )
	{
		const iCoord2D &maxConcavityPoint = contourSlices[i].maxConcavityPoint();
		if ( maxConcavityPoint != invalidICoord2D )
		{
			_maxConcavityPointsSerie.append(rCoord2D(i,fmod(contourSlices[i].maxSupportPoint().angle(maxConcavityPoint)+TWO_PI,TWO_PI)*RAD_TO_DEG_FACT));
		}
		const iCoord2D &minConcavityPoint = contourSlices[i].minConcavityPoint();
		if ( minConcavityPoint != invalidICoord2D )
		{
			_minConcavityPointsSerie.append(rCoord2D(i,fmod(contourSlices[i].minSupportPoint().angle(minConcavityPoint)+TWO_PI,TWO_PI)*RAD_TO_DEG_FACT));
		}
	}
}
