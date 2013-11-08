#include "inc/billonalgorithms.h"

#include "inc/billon.h"
#include "inc/piechart.h"

namespace BillonAlgorithms
{
	qreal restrictedAreaMeansRadius(const Billon &billon, const uint &nbDirections, const int &intensityThreshold, const uint &minimumRadius, const uint &nbSlicesToIgnore )
	{
		Q_ASSERT_X( nbDirections>0 , "BillonTpl<T>::getRestrictedAreaMeansRadius", "nbPolygonPoints arguments equals to 0 => division by zero" );

		qreal radius = billon.n_cols/2.;
		if ( billon.hasPith() )
		{
			const int width = billon.n_cols;
			const int height = billon.n_rows;
			const int depth = billon.n_slices-nbSlicesToIgnore;
			const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbDirections);

			rCoord2D center, edge;
			rVec2D direction;
			qreal orientation, currentNorm;

			radius = width;
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
					while ( edge.x>0 && edge.y>0 && edge.x<width && edge.y<height && currentSlice.at(edge.y,edge.x) > intensityThreshold )
					{
						edge += direction;
					}
					currentNorm = rVec2D(edge-center).norm();
					if ( currentNorm < radius ) radius = currentNorm;
				}
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
					while ( edge.x>0. && edge.y>0. && edge.x<width && edge.y<height && currentSlice.at(edge.y,edge.x) >= intensityThreshold )
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
								  const qreal &length, const uint &nbSlices, const int &minIntensity )
	{
		const qreal zPithCoord = sliceInterval.mid();
		const rCoord2D &originPith = billon.pithCoord(zPithCoord);
		const uint angularRange = (angularInterval.max() + (angularInterval.max()>angularInterval.min() ? 0. : PieChartSingleton::getInstance()->nbSectors())) - angularInterval.min();

		// Dimensions des coupes tangentielles
		const uint width = sliceInterval.size()+1;
		const uint height = 2 * qTan(angularRange*PieChartSingleton::getInstance()->sectorAngle()/2.) * length;
		const int widthOnTWo = width/2.;
		const int heightOnTWo = height/2.;

		Billon * tangentialBillon = new Billon(width,height,nbSlices);
		tangentialBillon->fill(minIntensity);

		// Rotation autour de l'axe Y
		const qreal alpha = PI_ON_TWO;
		const qreal cosAlpha = qCos(alpha);
		const qreal sinAlpha = qSin(alpha);
		const arma::Mat<qreal>::fixed<3,3> rotationMatY = { cosAlpha, 0, -sinAlpha, 0, 1, 0, sinAlpha, 0, cosAlpha };

		// Rotation selon l'angle de la zone de nœuds
		const qreal beta = (angularInterval.min()+angularRange/2.)*PieChartSingleton::getInstance()->sectorAngle();
		const qreal cosBeta = qCos(beta);
		const qreal sinBeta = qSin(beta);
		const arma::Mat<qreal>::fixed<3,3> rotationMatX = { 1, 0, 0, 0, cosBeta, -sinBeta, 0, sinBeta, cosBeta };

		const arma::Mat<qreal>::fixed<3,3> rotationMat = rotationMatY*rotationMatX;

		// Vecteur de déplacement entre deux coupes tangentielles successives
		const arma::Col<qreal>::fixed<3> shiftStep = { 0., 0., length/(1.*nbSlices) };
		const arma::Col<qreal>::fixed<3> originShift = rotationMat * shiftStep;

		arma::Col<qreal>::fixed<3> origin = { originPith.x, originPith.y, zPithCoord };
		arma::Col<qreal>::fixed<3> initial, destination;
		initial(2) = 0.;

		for ( uint k=0 ; k<nbSlices ; ++k )
		{
			Slice &slice = tangentialBillon->slice(k);
			for ( int j=-heightOnTWo ; j<heightOnTWo ; ++j )
			{
				initial(1) = j;
				for ( int i=-widthOnTWo ; i<widthOnTWo ; ++i )
				{
					initial(0) = i;
					destination = (rotationMat * initial) + origin;
//					if ( (k==0||k==nbSlices-1) && (j==-heightOnTWo||j==heightOnTWo-1) && (i==-widthOnTWo||i==widthOnTWo-1) )
//						std::cout << "( " << i << ", " << j << ", " << k << " ) => ( " << destination(0) << ", " << destination(1) << ", " << destination(2) << " )" << std::endl;
					slice(j+heightOnTWo,i+widthOnTWo) =
							destination(0)>=0 && destination(0)<billon.n_cols && destination(1)>=0 && destination(1)<billon.n_rows && destination(2)>=0 && destination(2)<billon.n_slices ?
								billon(destination(1),destination(0),destination(2)) : minIntensity;
				}
			}
			origin += originShift;
		}

		return tangentialBillon;
	}

}
