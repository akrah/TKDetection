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
							 const Interval<qreal> &angularInterval, const uint &minimumDistanceFromContourOrigin )
{
	if ( !sliceIntervals.size() ) return;

	const Interval<uint> &sliceInterval = sliceIntervals[0];
	const uint nbSlices = sliceInterval.width()+1;
	const uint intervalStart = sliceInterval.min();
	const uint intervalEnd = sliceInterval.max();

	const bool minSupMax = angularInterval.min() > angularInterval.max();
	const qreal angleMax = minSupMax ? angularInterval.max() + TWO_PI : angularInterval.max();
	const qreal angleMin = angularInterval.min();
	const qreal angleMid = fmod(((angleMax + angleMin) / 2. + PI),TWO_PI);
	const qreal maxAngle = (minSupMax && angleMax < angleMid) ? angleMax+TWO_PI : angleMax;
	const qreal minAngle = angleMin;

	qreal currentAngle, currentPithAngle, previousAngle, nextAngle;

	resultBillon = Billon(billon,sliceInterval);
	_contourSlices.resize(nbSlices);

	for ( uint k=intervalStart ; k<=intervalEnd ; ++k )
	{
		ContourSlice &contourSlice = _contourSlices[k-intervalStart];

		qDebug() << QString("Calcul de la coupe de contour : %1/%2").arg(k+1-intervalStart).arg(nbSlices);
		contourSlice.computeStart( billon.slice(k), billon.pithCoord(k), intensityThreshold,
								   smoothingRadius, curvatureWidth, curvatureThreshold, minimumDistanceFromContourOrigin );

		if ( contourSlice.maxConcavityPointIndex() != -1 )
		{
			currentAngle = fmod(contourSlice.maxSupportPoint().angle(contourSlice.maxConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentAngle < angleMid ) currentAngle += TWO_PI;
			currentPithAngle = fmod(billon.pithCoord(k).angle(contourSlice.maxSupportPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentPithAngle < angleMid ) currentPithAngle += TWO_PI;
			if ( currentAngle < minAngle )
			{
				contourSlice.setMaxSupportPoint( rCoord2D( contourSlice.maxConcavityPoint().x-qCos(minAngle),
														   contourSlice.maxConcavityPoint().y-qSin(minAngle) ) );
			}
			else if ( currentAngle > currentPithAngle )
			{
				contourSlice.setMaxSupportPoint( billon.pithCoord(k) );
			}
		}
		if ( contourSlice.minConcavityPointIndex() != -1 )
		{
			currentAngle = fmod(contourSlice.minSupportPoint().angle(contourSlice.minConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentAngle < angleMid ) currentAngle += TWO_PI;
			currentPithAngle = fmod(billon.pithCoord(k).angle(contourSlice.minSupportPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentPithAngle < angleMid ) currentPithAngle += TWO_PI;
			if ( currentAngle > maxAngle )
			{
				contourSlice.setMinSupportPoint( rCoord2D( contourSlice.minConcavityPoint().x-qCos(maxAngle),
														   contourSlice.minConcavityPoint().y-qSin(maxAngle) ) );
			}
			else if ( currentAngle < currentPithAngle )
			{
				contourSlice.setMinSupportPoint( billon.pithCoord(k) );
			}
		}
	}

	qDebug() << QString("Optimisation de la coupe de contour : 1/%1").arg(nbSlices);
	_contourSlices[0].computeEnd( resultBillon.slice(0), billon.slice(intervalStart), intensityThreshold );
	for ( uint k=intervalStart+1 ; k<intervalEnd ; ++k )
	{
		ContourSlice &contourSlice = _contourSlices[k-intervalStart];
		ContourSlice &previousContourSlice = _contourSlices[k-1-intervalStart];
		ContourSlice &nextContourSlice = _contourSlices[k+1-intervalStart];

		qDebug() << QString("Optimisation de la coupe de contour : %1/%2").arg(k+1-intervalStart).arg(nbSlices);

		if ( contourSlice.maxConcavityPointIndex() != -1 && previousContourSlice.maxConcavityPointIndex() != -1 && nextContourSlice.maxConcavityPointIndex() != -1 )
		{
			currentAngle = fmod(contourSlice.maxSupportPoint().angle(contourSlice.maxConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentAngle < angleMid ) currentAngle += TWO_PI;
			previousAngle = fmod(previousContourSlice.maxSupportPoint().angle(previousContourSlice.maxConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && previousAngle < angleMid ) previousAngle += TWO_PI;
			nextAngle = fmod(nextContourSlice.maxSupportPoint().angle(nextContourSlice.maxConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && nextAngle < angleMid ) nextAngle += TWO_PI;
			if ( (previousAngle < currentAngle) == (nextAngle < currentAngle) )
			{
				contourSlice.setMaxSupportPoint( rCoord2D( contourSlice.maxConcavityPoint().x - qCos((previousAngle+nextAngle)/2.),
														   contourSlice.maxConcavityPoint().y - qSin((previousAngle+nextAngle)/2.) ) );
			}
		}
		if ( contourSlice.minConcavityPointIndex() != -1 && previousContourSlice.minConcavityPointIndex() != -1 && nextContourSlice.minConcavityPointIndex() != -1 )
		{
			currentAngle = fmod(contourSlice.minSupportPoint().angle(contourSlice.minConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && currentAngle < angleMid ) currentAngle += TWO_PI;
			previousAngle = fmod(previousContourSlice.minSupportPoint().angle(previousContourSlice.minConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && previousAngle < angleMid ) previousAngle += TWO_PI;
			nextAngle = fmod(nextContourSlice.minSupportPoint().angle(nextContourSlice.minConcavityPoint())+TWO_PI,TWO_PI);
			if ( minSupMax && nextAngle < angleMid ) nextAngle += TWO_PI;
			if ( (previousAngle < currentAngle) == (nextAngle < currentAngle) )
			{
				contourSlice.setMinSupportPoint( rCoord2D( contourSlice.minConcavityPoint().x - qCos((previousAngle+nextAngle)/2.),
														   contourSlice.minConcavityPoint().y - qSin((previousAngle+nextAngle)/2.) ) );
			}
		}

		contourSlice.computeEnd( resultBillon.slice(k-intervalStart), billon.slice(k), intensityThreshold );
	}
	qDebug() << QString("Optimisation de la coupe de contour : %1/%2").arg(nbSlices).arg(nbSlices);
	_contourSlices.last().computeEnd( resultBillon.slice(nbSlices-1), billon.slice(intervalEnd), intensityThreshold );
}
