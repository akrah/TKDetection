#include "inc/billonalgorithms.h"

#include "inc/billon.h"
#include "inc/piechart.h"

#include <QQuaternion>

namespace BillonAlgorithms
{
	qreal restrictedAreaMeansRadius(const Billon &billon, const uint &nbDirections, const int &intensityThreshold, const uint &minimumRadius, const uint &nbSlicesToIgnore )
	{
		Q_ASSERT_X( nbDirections>0 , "BillonTpl<T>::getRestrictedAreaMeansRadius", "nbPolygonPoints arguments equals to 0 => division by zero" );

		if ( !billon.hasPith() )
			return 1;

		const int &width = billon.n_cols;
		const int &height = billon.n_rows;
		const int depth = billon.n_slices-nbSlicesToIgnore;
		const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbDirections);

		rCoord2D center, edge;
		rVec2D direction;
		qreal orientation, currentNorm;

		qreal radius = width;
		for ( int k=nbSlicesToIgnore ; k<depth ; ++k )
		{
			const Slice &currentSlice = billon.slice(k);
			center.x = billon.pithCoord(k).x;
			center.y = billon.pithCoord(k).y;
			orientation = angleIncrement;
			while (orientation < TWO_PI)
			{
				orientation += angleIncrement;
				direction = rVec2D(qCos(orientation),qSin(orientation));
				edge = center + direction*minimumRadius;
				while ( edge.x>0 && edge.y>0 && edge.x<width && edge.y<height && currentSlice(edge.y,edge.x) > intensityThreshold )
				{
					edge += direction;
				}
				currentNorm = rVec2D(edge-center).norm();
				if ( currentNorm < radius ) radius = currentNorm;
			}
		}
		qDebug() << "Rayon de la boite englobante (en pixels) : " << radius;
		return radius;
	}

	QVector<rCoord2D> restrictedAreaVertex( const Billon &billon, const Interval<uint> & sliceInterval, const uint & nbPolygonVertex, const int & intensityThreshold )
	{
		Q_ASSERT_X( nbPolygonVertex>0 , "BillonTpl<T>::getRestrictedAreaVertex", "nbPolygonVertex arguments equals to 0 => division by zero" );

		QVector<rCoord2D> vectAllVertex;
		if ( billon.hasPith() )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonVertex);
			rCoord2D edge, center;
			rVec2D direction;
			qreal orientation;
			for ( uint indexSlice = sliceInterval.min() ; indexSlice<=sliceInterval.max() ; ++indexSlice )
			{
				const Slice & currentSlice = billon.slice(indexSlice);
				center.x = billon.pithCoord(indexSlice).x;
				center.y = billon.pithCoord(indexSlice).y;
				orientation = 0.;
				while (orientation < TWO_PI)
				{
					orientation += angleIncrement;
					direction = rVec2D(qCos(orientation),qSin(orientation));
					edge = center + direction*30;
					while ( edge.x>0. && edge.y>0. && edge.x<width && edge.y<height && currentSlice(edge.y,edge.x) >= intensityThreshold )
					{
						edge += direction;
					}
					vectAllVertex.push_back(edge);
				}
			}
		}
		return vectAllVertex;
	}

}
