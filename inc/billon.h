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
	BillonTpl( int width, int height, int depth );
	BillonTpl( const BillonTpl &billon );

	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;

	void setMinValue( T value );
	void setMaxValue( T value );
	void setVoxelSize( qreal width, qreal height, qreal depth );

	QVector<rCoord2D> getAllRestrictedAreaVertex( const int &nbPolygonsPoints, const int &threshold, const Marrow *marrow = 0 ) const;
	QVector<rCoord2D> getRestrictedAreaVertex( const int &nbPolygonsPoints, const int &threshold, unsigned int minSlice, unsigned int maxSlice, const Marrow *marrow = 0) const;
	qreal getRestrictedAreaMeansRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const;

	iCoord2D findNearestPointOfThePith( const iCoord2D &center, const int &sliceNumber, const int &threshold ) const;
	QVector<iCoord2D> extractContour( const iCoord2D &center, const int &sliceNumber, int threshold, iCoord2D startPoint = iCoord2D(-1,-1) ) const;

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
template< typename T > BillonTpl<T>::BillonTpl( int width, int height, int depth ) : arma::Cube<T>(height,width,depth), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}
template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl &billon ) : arma::Cube<T>(billon), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

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
void BillonTpl<T>::setMinValue( T value )
{
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( T value )
{
	_maxValue = value;
}

template< typename T >
void BillonTpl<T>::setVoxelSize( qreal width, qreal height, qreal depth )
{
	_voxelWidth = width;
	_voxelHeight = height;
	_voxelDepth = depth;
}

template< typename T >
QVector<rCoord2D> BillonTpl<T>::getAllRestrictedAreaVertex( const int &nbPolygonPoints, const int &threshold, const Marrow *marrow ) const
{
	 QVector<rCoord2D> vectAllVertex;
	 const int nbSlices = this->n_slices;
	 const int thresholdRestrict = threshold-1;
	 for ( int indexSlice = 0 ; indexSlice<nbSlices ; ++indexSlice ) {
		 const arma::Mat<T> &currentSlice = this->slice(indexSlice);

		 const int sliceWidth = currentSlice.n_cols;
		 const int sliceHeight = currentSlice.n_rows;
		 const int xCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).x:sliceWidth/2;
		 const int yCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).y:sliceHeight/2;

		 qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
		 orientation = 0.;
		 for ( int i=0 ; i<nbPolygonPoints ; ++i ) {
			 orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			 cosAngle = qCos(orientation);
			 sinAngle = -qSin(orientation);
			 xEdge = xCenter + 5*cosAngle;
			 yEdge = yCenter + 5*sinAngle;
			 while ( xEdge>0. && yEdge>0. && xEdge<sliceWidth && yEdge<sliceHeight && currentSlice.at(yEdge,xEdge) > thresholdRestrict ) {
					 xEdge += cosAngle;
					 yEdge += sinAngle;
			 }
			 vectAllVertex.push_back(rCoord2D(xEdge,yEdge));
		 }
	 }
	return vectAllVertex;
}


template< typename T >
QVector<rCoord2D> BillonTpl<T>::getRestrictedAreaVertex( const int &nbPolygonPoints, const int &threshold,
							 unsigned int minSlice, unsigned int maxSlice, const Marrow *marrow ) const {
	 QVector<rCoord2D> vectAllVertex;
	 const int thresholdRestrict = threshold-1;
	 for ( uint indexSlice = minSlice ; indexSlice<=maxSlice ; ++indexSlice ) {
		 const arma::Mat<T> &currentSlice = this->slice(indexSlice);

		 const int sliceWidth = currentSlice.n_cols;
		 const int sliceHeight = currentSlice.n_rows;
		 const int xCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).x:sliceWidth/2;
		 const int yCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).y:sliceHeight/2;

		 qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
		 orientation = 0.;
		 for ( int i=0 ; i<nbPolygonPoints ; ++i ) {
			 orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			 cosAngle = qCos(orientation);
			 sinAngle = -qSin(orientation);
			 xEdge = xCenter + 5*cosAngle;
			 yEdge = yCenter + 5*sinAngle;
			 while ( xEdge>0. && yEdge>0. && xEdge<sliceWidth && yEdge<sliceHeight && currentSlice.at(yEdge,xEdge) > thresholdRestrict ) {
					 xEdge += cosAngle;
					 yEdge += sinAngle;
			 }
			 vectAllVertex.push_back(rCoord2D(xEdge,yEdge));
		 }
	 }
	return vectAllVertex;
}

template< typename T >
qreal BillonTpl<T>::getRestrictedAreaMeansRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const
{
	qreal xEdge, yEdge, orientation, cosAngle, sinAngle, radius;
	int i,k;

	const int width = this->n_cols;
	const int height = this->n_rows;
	const int depth = this->n_slices;

	radius = 0.;
	for ( k=0 ; k<depth ; ++k ) {
		const arma::Mat<T> &currentSlice = this->slice(k);
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().min()).x:width/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().min()).y:height/2;

		orientation = 0.;
		for ( i=0 ; i<nbPolygonPoints ; ++i )
		{
			orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			cosAngle = qCos(orientation);
			sinAngle = -qSin(orientation);
			xEdge = xCenter + 10*cosAngle;
			yEdge = yCenter + 10*sinAngle;
			while ( xEdge>0 && yEdge>0 && xEdge<width && yEdge<height && currentSlice.at(yEdge,xEdge) > intensityThreshold )
			{
				xEdge += cosAngle;
				yEdge += sinAngle;
			}
			xEdge -= xCenter;
			yEdge -= yCenter;
			radius += qSqrt( xEdge*xEdge + yEdge*yEdge )/static_cast<qreal>(nbPolygonPoints);
		}
	}

	radius/=depth;
	qDebug() << "Rayon de la boite englobante (en pixels) : " << radius;
	return radius;
}

template < typename T >
iCoord2D BillonTpl<T>::findNearestPointOfThePith( const iCoord2D &center, const int &sliceNumber, const int &threshold ) const
{
	// Find the pixel closest to the pith
	const arma::Mat<T> &currentSlice = this->slice(sliceNumber);
	const int width = this->n_cols;
	const int height = this->n_rows;
	const int xCenter = center.x;
	const int yCenter = center.y;
	const int radiusMax = qMin( qMin(xCenter,width-xCenter), qMin(yCenter,height-yCenter) );

	iCoord2D position;
	bool edgeFind = false;
	int currentRadius, x, y, d;

	currentRadius = 1;
	while ( !edgeFind && currentRadius < radiusMax )
	{
		x = 0;
		y = currentRadius;
		d = currentRadius - 1;
		while ( y>=x && !edgeFind )
		{
			edgeFind = true;
			if ( currentSlice.at( yCenter+y, xCenter+x ) > threshold )
			{
				position.x = xCenter+x;
				position.y = yCenter+y;
			}
			else if ( currentSlice.at( yCenter+y, xCenter-x ) > threshold )
			{
				position.x = xCenter-x;
				position.y = yCenter+y;
			}
			else if ( currentSlice.at( yCenter+x, xCenter+y ) > threshold )
			{
				position.x = xCenter+y;
				position.y = yCenter+x;
			}
			else if ( currentSlice.at( yCenter+x, xCenter-y ) > threshold )
			{
				position.x = xCenter-y;
				position.y = yCenter+x;
			}
			else if ( currentSlice.at( yCenter-y, xCenter+x ) > threshold )
			{
				position.x = xCenter+x;
				position.y = yCenter-y;
			}
			else if ( currentSlice.at( yCenter-y, xCenter-x ) > threshold )
			{
				position.x = xCenter-x;
				position.y = yCenter-y;
			}
			else if ( currentSlice.at( yCenter-x, xCenter+y ) > threshold )
			{
				position.x = xCenter+y;
				position.y = yCenter-x;
			}
			else if ( currentSlice.at( yCenter-x, xCenter-y ) > threshold )
			{
				position.x = xCenter-y;
				position.y = yCenter-x;
			}
			else
			{
				edgeFind = false;
				if ( d >= 2*x )
				{
					d -= 2*x;
					x++;
				}
				else if ( d <= 2*(currentRadius-y) )
				{
					d += 2*y;
					y--;
				}
				else
				{
					d += 2*(y-x);
					y--;
					x++;
				}
			}
		}
		currentRadius++;
	}

	if ( edgeFind ) {
		qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
	}
	else {
		qDebug() << "Aucun pixel et donc aucune composante connexe";
		position.x = -1;
		position.y = -1;
	}

	return position;
}

template< typename T >
QVector<iCoord2D> BillonTpl<T>::extractContour( const iCoord2D &center, const int &sliceNumber, int threshold, iCoord2D startPoint ) const
{
	QVector<iCoord2D> contourPoints;

	if ( startPoint == iCoord2D(-1,-1) )
	{
		startPoint = findNearestPointOfThePith( center, sliceNumber, threshold );
	}

	if ( startPoint != iCoord2D(-1,-1) )
	{
		const arma::Mat<T> &currentSlice = this->slice(sliceNumber);
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
