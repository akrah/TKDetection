#include "inc/billonalgorithms.h"

#include "inc/billon.h"

namespace BillonAlgorithms
{
	iCoord2D findNearestPointOfThePith( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold )
	{
		// Find the pixel closest to the pith
		const int width = slice.n_cols;
		const int height = slice.n_rows;
		const int radiusMax = qMin( qMin(sliceCenter.x,width-sliceCenter.x), qMin(sliceCenter.y,height-sliceCenter.y) );

		iCoord2D position, circlePoint;
		bool edgeFind = false;
		int currentRadius, d;

		currentRadius = 1;
		// Using Andres circle algorithm
		while ( !edgeFind && currentRadius < radiusMax )
		{
			circlePoint.x = 0;
			circlePoint.y = currentRadius;
			d = currentRadius - 1;
			while ( circlePoint.y>=circlePoint.x && !edgeFind )
			{
				edgeFind = true;
				if ( slice.at( sliceCenter.y+circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y+circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y+circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.y;
					position.y = sliceCenter.y+circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.y, sliceCenter.x+circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.y, sliceCenter.x-circlePoint.x ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.x;
					position.y = sliceCenter.y-circlePoint.y;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.x, sliceCenter.x+circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x+circlePoint.y;
					position.y = sliceCenter.y-circlePoint.x;
				}
				else if ( slice.at( sliceCenter.y-circlePoint.x, sliceCenter.x-circlePoint.y ) > intensityThreshold )
				{
					position.x = sliceCenter.x-circlePoint.y;
					position.y = sliceCenter.y-circlePoint.x;
				}
				else
				{
					edgeFind = false;
					if ( d >= 2*circlePoint.x )
					{
						d -= 2*circlePoint.x;
						circlePoint.x++;
					}
					else if ( d <= 2*(currentRadius-circlePoint.y) )
					{
						d += 2*circlePoint.y;
						circlePoint.y--;
					}
					else
					{
						d += 2*(circlePoint.y-circlePoint.x);
						circlePoint.y--;
						circlePoint.x++;
					}
				}
			}
			currentRadius++;
		}

		if ( edgeFind ) {
			qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
			return position;
		}
		else {
			qDebug() << "Aucun pixel et donc aucune composante connexe";
			return iCoord2D(-1,-1);
		}
	}

	QVector<iCoord2D> extractContour( const Slice &slice, const iCoord2D & sliceCenter, int intensityThreshold, iCoord2D startPoint )
	{
		QVector<iCoord2D> contourPoints;
		if ( startPoint == iCoord2D(-1,-1) )
		{
			startPoint = findNearestPointOfThePith( slice, sliceCenter, intensityThreshold );
		}
		if ( startPoint != iCoord2D(-1,-1) )
		{
			iCoord2D currentPos(startPoint);
			QVector<iCoord2D> mask(8);
			qreal orientation;
			int interdit, j;

			orientation = sliceCenter.angle(startPoint);
			interdit = orientation*8./TWO_PI;
			interdit = (interdit+4)%8;
			intensityThreshold++;
			do
			{
				contourPoints.append(currentPos);
				mask[0].x = mask[1].x = mask[7].x = currentPos.x+1;
				mask[2].x = mask[6].x = currentPos.x;
				mask[3].x = mask[4].x = mask[5].x = currentPos.x-1;
				mask[1].y = mask[2].y = mask[3].y = currentPos.y+1;
				mask[0].y = mask[4].y = currentPos.y;
				mask[5].y = mask[6].y = mask[7].y = currentPos.y-1;
				j = (interdit+1)%8;
				while ( slice.at(mask[j%8].y,mask[j%8].x) < intensityThreshold && j < interdit+8 ) ++j;
				currentPos = mask[j%8];
				interdit = (j+4)%8;
			}
			while ( startPoint != currentPos );
		}

		return contourPoints;
	}

	qreal getRestrictedAreaMeansRadius( const Billon &billon, const Pith &pith, const uint &nbPolygonPoints, int intensityThreshold )
	{
		Q_ASSERT_X( nbPolygonPoints>0 , "BillonTpl<T>::getRestrictedAreaMeansRadius", "nbPolygonPoints arguments equals to 0 => division by zero" );

		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const int depth = billon.n_slices;
		const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonPoints);

		rCoord2D center, edge;
		rVec2D direction;
		qreal orientation, radius;

		radius = 0.;
		for ( int k=0 ; k<depth ; ++k )
		{
			const Slice &currentSlice = billon.slice(k);
			center.x = pith[k].x;
			center.y = pith[k].y;
			orientation = 0.;
			while (orientation < TWO_PI)
			{
				orientation += angleIncrement;
				direction = rVec2D(qCos(orientation),qSin(orientation));
				edge = center + direction*30;
				while ( edge.x>0 && edge.y>0 && edge.x<width && edge.y<height && currentSlice.at(edge.y,edge.x) > intensityThreshold )
				{
					edge += direction;
				}
				edge -= center;
				radius += rVec2D(edge).norm()/nbPolygonPoints;
			}
		}

		radius/=depth;
		qDebug() << "Rayon de la boite englobante (en pixels) : " << radius;
		return radius;
	}

	QVector<rCoord2D> getRestrictedAreaVertex( const Billon &billon, const Pith & pith, const Interval<uint> & sliceInterval, const uint & nbPolygonPoints, const int & intenstyThreshold )
	{
		Q_ASSERT_X( nbPolygonPoints>0 , "BillonTpl<T>::getRestrictedAreaVertex", "nbPolygonPoints arguments equals to 0 => division by zero" );

		QVector<rCoord2D> vectAllVertex;
		const int width = billon.n_cols;
		const int height = billon.n_rows;
		const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonPoints);
		rCoord2D edge, center;
		rVec2D direction;
		qreal orientation;
		for ( uint indexSlice = sliceInterval.min() ; indexSlice<=sliceInterval.max() ; ++indexSlice )
		{
			const Slice & currentSlice = billon.slice(indexSlice);
			center.x = pith[indexSlice].x;
			center.y = pith[indexSlice].y;
			orientation = 0.;
			while (orientation < TWO_PI)
			{
				orientation += angleIncrement;
				direction = rVec2D(qCos(orientation),qSin(orientation));
				edge = center + direction*30;
				while ( edge.x>0. && edge.y>0. && edge.x<width && edge.y<height && currentSlice.at(edge.y,edge.x) >= intenstyThreshold )
				{
					edge += direction;
				}
				vectAllVertex.push_back(edge);
			}
		}
		return vectAllVertex;
	}
}
