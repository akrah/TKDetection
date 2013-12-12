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

	Billon * tangentialTransform( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &angularInterval,
								  const int &minIntensity, const bool &trilinearInterpolation, const qreal &linearInterpolationCoeff )
	{
		const qreal zPithCoord = sliceInterval.mid();
		const rCoord2D &originPith = billon.pithCoord(zPithCoord);
		const uint angularRange = (angularInterval.max() + (angularInterval.isValid() ? 0. : PieChartSingleton::getInstance()->nbSectors())) - angularInterval.min();

		const qreal bisectorOrientation = (angularInterval.min()+angularRange/2.)*PieChartSingleton::getInstance()->angleStep();
		const qreal cosBisector = qCos(bisectorOrientation);
		const qreal sinBisector = qSin(bisectorOrientation);

		// Dimensions des coupes tangentielles
		/* Recherche de l'écorce dans la direction de la bissectrice du secteur angulaire sur la coupe au milieu de l'intervalle de coupes */
		const Slice &midSlice = billon.slice(zPithCoord);
		const rVec2D direction(cosBisector,sinBisector);
		rCoord2D edge = originPith + direction*30;
		while ( edge.x>0 && edge.y>0 && edge.x<billon.n_cols && edge.y<billon.n_rows && midSlice(edge.y,edge.x) > minIntensity )
		{
			edge += direction;
		}
		const qreal depth = rVec2D(edge-originPith).norm();

		/* Hauteur et largeur des coupes transversales */
		const int &billonWidthMinusOne = billon.n_cols-1;
		const int &billonHeightMinusOne = billon.n_rows-1;
		const int &billonDepthMinusOne = billon.n_slices-1;

		/* Hauteur et largeur des coupes tangentielles */
		const uint width = sliceInterval.size()+1;
		const uint height = 2 * qTan(angularRange*PieChartSingleton::getInstance()->angleStep()/2.) * depth;
		const int widthOnTwo = qFloor((width-1)/2.);
		const int heightOnTwo = qFloor(height/2.);
		const int heightOnTwoMinusOne = heightOnTwo-1;

		const uint nbSlices = qRound(depth);

		// Inversion width et height pour correspondre à la ratation de 90°
		Billon * tangentialBillon = new Billon(height,width,nbSlices);
		//tangentialBillon->setVoxelSize( billon.voxelWidth(), billon.voxelDepth(), billon.voxelWidth() );
		tangentialBillon->setVoxelSize( qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)),
										billon.voxelDepth(),
										(billon.voxelWidth()*billon.voxelHeight())/qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)) );
		tangentialBillon->fill(minIntensity);

		// Rotation autour de l'axe Y
		const qreal alpha = PI_ON_TWO;
		const QQuaternion quaterY = QQuaternion::fromAxisAndAngle( 0, 1, 0, alpha*RAD_TO_DEG_FACT );

		// Rotation selon l'angle de la zone de nœuds
		const QQuaternion quaterZ = QQuaternion::fromAxisAndAngle( 0, 0, 1, bisectorOrientation*RAD_TO_DEG_FACT );

		// Combinaisons des rotations
		const QQuaternion quaterRot = quaterZ * quaterY;

		// Vecteur de déplacement entre deux coupes tangentielles successives
		const QVector3D shiftStep = quaterRot.rotatedVector( QVector3D( 0., 0., 1. ) );

		QVector3D origin( originPith.x, originPith.y, zPithCoord );
		QVector3D initial, destination;

		const qreal semiKnotAreaHeightCoeff = heightOnTwo / static_cast<qreal>( nbSlices );
		const qreal originLinearInterpolationCoeff = 1.-linearInterpolationCoeff;
		int i, j, jStart, jEnd;
		qreal semiKnotAreaHeight;

		int x0,y0,z0;
		qreal x0Dist, y0Dist, z0Dist;
		qreal xFrontTop, xFrontBottom, xBackTop, xBackBottom, yFront, yBack;

		jStart = jEnd = 0;
		initial.setZ(0.);
		for ( uint k=0 ; k<nbSlices ; ++k )
		{
			Slice &slice = tangentialBillon->slice(k);
			semiKnotAreaHeight = k*semiKnotAreaHeightCoeff;
			jEnd = qMin(qRound(semiKnotAreaHeight),heightOnTwoMinusOne);
			jStart = -jEnd;
			for ( j=jStart ; j<=jEnd ; ++j )
			{
				initial.setY(j);
				for ( i=-widthOnTwo ; i<=widthOnTwo ; ++i )
				{
					initial.setX(i);
					destination = quaterRot.rotatedVector(initial) + origin;
					x0 = qFloor(destination.y());
					y0 = qFloor(destination.x());
					z0 = qFloor(destination.z());
					if ( x0>0 && x0<billonWidthMinusOne &&
						 y0>0 && y0<billonHeightMinusOne &&
						 z0>0 && z0<billonDepthMinusOne )
					{
						if ( trilinearInterpolation )
						{
							x0Dist = destination.y()-x0;
							y0Dist = destination.x()-y0;
							z0Dist = destination.z()-z0;
							xFrontTop = (1.-x0Dist)*billon(x0-1,y0+1,z0-1) + x0Dist*billon(x0+1,y0+1,z0-1);
							xFrontBottom = (1.-x0Dist)*billon(x0-1,y0-1,z0-1) + x0Dist*billon(x0+1,y0-1,z0-1);
							xBackTop = (1.-x0Dist)*billon(x0-1,y0+1,z0+1) + x0Dist*billon(x0+1,y0+1,z0+1);
							xBackBottom = (1.-x0Dist)*billon(x0-1,y0-1,z0+1) + x0Dist*billon(x0+1,y0-1,z0+1);
							yFront = (1.-y0Dist)*xFrontBottom + y0Dist*xFrontTop;
							yBack = (1.-y0Dist)*xBackBottom + y0Dist*xBackTop;
							// Rotation de 90° dans le sens horaire pour correspondre à l'orientation de l'article
							slice(i+widthOnTwo,heightOnTwoMinusOne-j) = originLinearInterpolationCoeff * billon(x0,y0,z0)
																		  + linearInterpolationCoeff * ((1.-z0Dist)*yFront + z0Dist*yBack);
						}
						else
						{
							// Rotation de 90° dans le sens horaire pour correspondre à l'orientation de l'article
							slice(i+widthOnTwo,heightOnTwoMinusOne-j) =	billon(x0,y0,z0);
						}
					}
				}
			}
			origin += shiftStep;
		}

		return tangentialBillon;
	}
}
