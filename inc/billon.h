#ifndef BILLON_H
#define BILLON_H

#include "global.h"
#include "marrow.h"

#include <QDebug>
#include <armadillo>

#include <QPolygon>

/*######################################################
  # DECLARATION
  ######################################################*/

template< typename T >
class BillonTpl : public arma::Cube<T>
{
public:
	BillonTpl();
	BillonTpl( const int & width, const int & height, const int & depth );
	BillonTpl( const BillonTpl<T> &billon );

	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

	QVector<rCoord2D> getRestrictedAreaVertex( const Marrow &marrow, const Interval<uint> &sliceInterval, const uint &nbPolygonPoints, const int &intenstyThreshold ) const;
	qreal getRestrictedAreaMeansRadius( const Marrow &marrow, const uint &nbPolygonPoints, int intensityThreshold ) const;

	iCoord2D findNearestPointOfThePith( const iCoord2D &center, const uint &sliceIndex, const int &intensityThreshold ) const;
	QVector<iCoord2D> extractContour( const iCoord2D &center, const uint &sliceIndex, int threshold, iCoord2D startPoint = iCoord2D(-1,-1) ) const;

protected:
	T _minValue;        // Valeur minimum à considérer (mais une valeur PEUT être plus petite)
	T _maxValue;        // Valeur maximum à considérer (mais une valeur PEUT être plus grande)
	qreal _voxelWidth;  // Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;  // Profondeur d'un voxel en cm
};

/*######################################################
  # INSTANCIATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template< typename T > BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}
template< typename T > BillonTpl<T>::BillonTpl( const int & width, const int & height, const int & depth ) : arma::Cube<T>(height,width,depth), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}
template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl<T> & billon ) : arma::Cube<T>(billon), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

/**********************************
 * Public getters
 **********************************/

template< typename T >
T BillonTpl<T>::minValue() const
{
	return _minValue;
}

template< typename T >
T BillonTpl<T>::maxValue() const
{
	return _maxValue;
}

template< typename T >
qreal BillonTpl<T>::voxelWidth() const
{
	return _voxelWidth;
}

template< typename T >
qreal BillonTpl<T>::voxelHeight() const
{
	return _voxelHeight;
}

template< typename T >
qreal BillonTpl<T>::voxelDepth() const
{
	return _voxelDepth;
}

/**********************************
 * Public setters
 **********************************/

template< typename T >
void BillonTpl<T>::setMinValue( const T & value )
{
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T & value )
{
	_maxValue = value;
}

template< typename T >
void BillonTpl<T>::setVoxelSize( const qreal & width, const qreal & height, const qreal & depth )
{
	_voxelWidth = width;
	_voxelHeight = height;
	_voxelDepth = depth;
}

template< typename T >
QVector<rCoord2D> BillonTpl<T>::getRestrictedAreaVertex( const Marrow & marrow, const Interval<uint> & sliceInterval, const uint & nbPolygonPoints, const int & intenstyThreshold ) const
{
	Q_ASSERT_X( nbPolygonPoints>0 , "BillonTpl<T>::getRestrictedAreaVertex", "nbPolygonPoints arguments equals to 0 => division by zero" );

	QVector<rCoord2D> vectAllVertex;
	const int width = this->n_cols;
	const int height = this->n_rows;
	const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonPoints);
	rCoord2D edge, center;
	rVec2D direction;
	qreal orientation;
	for ( uint indexSlice = sliceInterval.min() ; indexSlice<=sliceInterval.max() ; ++indexSlice )
	{
		const arma::Mat<T> & currentSlice = this->slice(indexSlice);
		center.x = marrow.at(indexSlice).x;
		center.y = marrow.at(indexSlice).y;
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

template <typename T>
qreal BillonTpl<T>::getRestrictedAreaMeansRadius( const Marrow &marrow, const uint &nbPolygonPoints, int intensityThreshold ) const
{
	Q_ASSERT_X( nbPolygonPoints>0 , "BillonTpl<T>::getRestrictedAreaMeansRadius", "nbPolygonPoints arguments equals to 0 => division by zero" );

	const int width = this->n_cols;
	const int height = this->n_rows;
	const int depth = this->n_slices;
	const qreal angleIncrement = TWO_PI/static_cast<qreal>(nbPolygonPoints);

	rCoord2D center, edge;
	rVec2D direction;
	qreal orientation, radius;

	radius = 0.;
	for ( int k=0 ; k<depth ; ++k ) {
		const arma::Mat<T> &currentSlice = this->slice(k);
		center.x = marrow.at(k).x;
		center.y = marrow.at(k).y;
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

template <typename T>
iCoord2D BillonTpl<T>::findNearestPointOfThePith( const iCoord2D &center, const uint &sliceIndex, const int &intensityThreshold ) const
{
	Q_ASSERT_X( sliceIndex<this->n_slices , "BillonTpl<T>::findNearestPointOfThePith", "sliceIndex is biggest than slice number" );

	// Find the pixel closest to the pith
	const arma::Mat<T> &currentSlice = this->slice(sliceIndex);
	const int width = this->n_cols;
	const int height = this->n_rows;
	const int radiusMax = qMin( qMin(center.x,width-center.x), qMin(center.y,height-center.y) );

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
			if ( currentSlice.at( center.y+circlePoint.y, center.x+circlePoint.x ) > intensityThreshold )
			{
				position.x = center.x+circlePoint.x;
				position.y = center.y+circlePoint.y;
			}
			else if ( currentSlice.at( center.y+circlePoint.y, center.x-circlePoint.x ) > intensityThreshold )
			{
				position.x = center.x-circlePoint.x;
				position.y = center.y+circlePoint.y;
			}
			else if ( currentSlice.at( center.y+circlePoint.x, center.x+circlePoint.y ) > intensityThreshold )
			{
				position.x = center.x+circlePoint.y;
				position.y = center.y+circlePoint.x;
			}
			else if ( currentSlice.at( center.y+circlePoint.x, center.x-circlePoint.y ) > intensityThreshold )
			{
				position.x = center.x-circlePoint.y;
				position.y = center.y+circlePoint.x;
			}
			else if ( currentSlice.at( center.y-circlePoint.y, center.x+circlePoint.x ) > intensityThreshold )
			{
				position.x = center.x+circlePoint.x;
				position.y = center.y-circlePoint.y;
			}
			else if ( currentSlice.at( center.y-circlePoint.y, center.x-circlePoint.x ) > intensityThreshold )
			{
				position.x = center.x-circlePoint.x;
				position.y = center.y-circlePoint.y;
			}
			else if ( currentSlice.at( center.y-circlePoint.x, center.x+circlePoint.y ) > intensityThreshold )
			{
				position.x = center.x+circlePoint.y;
				position.y = center.y-circlePoint.x;
			}
			else if ( currentSlice.at( center.y-circlePoint.x, center.x-circlePoint.y ) > intensityThreshold )
			{
				position.x = center.x-circlePoint.y;
				position.y = center.y-circlePoint.x;
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

template< typename T >
QVector<iCoord2D> BillonTpl<T>::extractContour( const iCoord2D & center, const uint & sliceIndex, int threshold, iCoord2D startPoint ) const
{
	Q_ASSERT_X( sliceIndex<this->n_slices , "BillonTpl<T>::extractContour", "sliceIndex is biggest than slice number" );

	QVector<iCoord2D> contourPoints;
	if ( startPoint == iCoord2D(-1,-1) )
	{
		startPoint = findNearestPointOfThePith( center, sliceIndex, threshold );
	}
	if ( startPoint != iCoord2D(-1,-1) )
	{
		const arma::Mat<T> &currentSlice = this->slice(sliceIndex);
		iCoord2D currentPos(startPoint);
		QVector<iCoord2D> mask(8);
		qreal orientation;
		int interdit, j;

		orientation = center.angle(startPoint);
		interdit = orientation*8./TWO_PI;
		interdit = (interdit+4)%8;
		threshold++;
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
			while ( currentSlice.at(mask[j%8].y,mask[j%8].x) < threshold && j < interdit+8 ) ++j;
			currentPos = mask[j%8];
			interdit = (j+4)%8;
		}
		while ( startPoint != currentPos );
	}

	return contourPoints;
}

#endif // BILLON_H
