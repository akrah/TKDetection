#include "inc/contourbillon.h"

#include "inc/contourslice.h"
#include "inc/curvaturehistogram.h"
#include "inc/nearestpointshistogram.h"

ContourBillon::ContourBillon()
{
}

ContourBillon::~ContourBillon()
{
}

const QVector<ContourSlice> &ContourBillon::contourSlices() const
{
	return _contourSlices;
}

const ContourSlice &ContourBillon::contourSlice( const uint &sliceIndex ) const
{
	Q_ASSERT_X( sliceIndex<static_cast<uint>(_contourSlices.size()) , "ContourCurveBillon::contour", "sliceIndex out of bounds" );
	return _contourSlices[sliceIndex];
}

void ContourBillon::clear()
{
	_contourSlices.clear();
}

bool ContourBillon::isEmpty()
{
	return _contourSlices.isEmpty();
}

void ContourBillon::compute( Billon &resultBillon, const Billon &billon, const int &intensityThreshold, const int &smoothingRadius,
							 const int &curvatureWidth, const qreal &curvatureThreshold, const QVector< Interval<uint> > &sliceIntervals,
							 const Interval<qreal> &angularInterval )
{
	resultBillon = billon;
	_contourSlices.resize(billon.n_slices);

	const qreal &angleMax = angularInterval.max();
	const qreal &angleMin = angularInterval.min();

	qreal currentMaxAngle, currentMinAngle;

	QVector< Interval<uint> >::ConstIterator intervalsIterator;
	for ( intervalsIterator = sliceIntervals.constBegin() ; intervalsIterator != sliceIntervals.constEnd() ; ++intervalsIterator )
	{
		const uint nbSlices = (*intervalsIterator).width()+1;
		const uint intervalStart = (*intervalsIterator).min();
		const uint intervalEnd = (*intervalsIterator).max()+1;
		for ( uint k=intervalStart ; k<intervalEnd ; ++k )
		{
			ContourSlice &contourSlice = _contourSlices[k];

			qDebug() << QString("Calcul de la coupe de contour : %1/%2").arg(k+1-intervalStart).arg(nbSlices);
			contourSlice.compute( resultBillon.slice(k), billon.slice(k), billon.pithCoord(k), intensityThreshold,
								  smoothingRadius, curvatureWidth, curvatureThreshold );

			if ( contourSlice.maxConcavityPointIndex() != -1 )
			{
				currentMaxAngle = fmod(contourSlice.maxSupportPoint().angle(contourSlice.maxConcavityPoint())+TWO_PI,TWO_PI);
				if ( currentMaxAngle < angleMax-PI_ON_TWO )
				{
					contourSlice.setMaxConcavityPointIndex(-1);
				}
				else if ( currentMaxAngle > angleMax )
				{
					contourSlice.setMaxSupportPoint( rCoord2D( contourSlice.maxConcavityPoint().x - qCos(angleMax), contourSlice.maxConcavityPoint().y - qSin(angleMax) ) );
				}
			}
			if ( contourSlice.minConcavityPointIndex() != -1 )
			{
				currentMinAngle = fmod(contourSlice.minSupportPoint().angle(contourSlice.minConcavityPoint())+TWO_PI,TWO_PI);
				if ( currentMinAngle > angleMin+PI_ON_TWO )
				{
					contourSlice.setMinConcavityPointIndex(-1);
				}
				else if ( currentMinAngle < angleMin )
				{
					contourSlice.setMinSupportPoint( rCoord2D( contourSlice.minConcavityPoint().x - qCos(angleMin), contourSlice.minConcavityPoint().y - qSin(angleMin) ) );
				}
			}
		}
	}
}
