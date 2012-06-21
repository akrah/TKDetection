#ifndef BILLON_H
#define BILLON_H

#include "global.h"
#include "marrow.h"

#include <QDebug>
#include <armadillo>

#include <QPolygon>

template< typename T >
class BillonTpl : public arma::Cube<T>
{
public:
	BillonTpl();
	BillonTpl( const int &width, const int &height, const int &depth );
	BillonTpl( const BillonTpl &billon );

	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

	QVector<rCoord2D> getAllRestrictedAreaVertex( const int &nbPolygonsPoints, const int &threshold, const Marrow *marrow = 0 ) const;
	qreal getRestrictedAreaBoudingBoxRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const;
	qreal getRestrictedAreaMeansRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const;

protected:
	T _minValue;
	T _maxValue;
	qreal _voxelWidth;	// Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;	// Profondeur d'un voxel en cm
};

template< typename T >
BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _minValue(static_cast<T>(0)), _maxValue(static_cast<T>(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int &width, const int &height, const int &depth ) : arma::Cube<T>(height,width,depth), _minValue(static_cast<T>(0)), _maxValue(static_cast<T>(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl &billon ) : arma::Cube<T>(billon), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

template< typename T >
T BillonTpl<T>::minValue() const {
	return _minValue;
}

template< typename T >
T BillonTpl<T>::maxValue() const {
	return _maxValue;
}

template< typename T >
qreal BillonTpl<T>::voxelWidth() const {
	return _voxelWidth;
}

template< typename T >
qreal BillonTpl<T>::voxelHeight() const {
	return _voxelHeight;
}

template< typename T >
qreal BillonTpl<T>::voxelDepth() const {
	return _voxelDepth;
}

template< typename T >
void BillonTpl<T>::setMinValue( const T &value ) {
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T &value ) {
	_maxValue = value;
}

template< typename T >
void BillonTpl<T>::setVoxelSize(const qreal &width, const qreal &height, const qreal &depth) {
	_voxelWidth = width;
	_voxelHeight = height;
	_voxelDepth = depth;
}

template< typename T >
QVector<rCoord2D> BillonTpl<T>::getAllRestrictedAreaVertex( const int &nbPolygonPoints, const int &threshold, const Marrow *marrow ) const {
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
qreal BillonTpl<T>::getRestrictedAreaBoudingBoxRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const
{
	QPolygon polygon(nbPolygonPoints);
	int polygonPoints[2*nbPolygonPoints+2];
	qreal xEdge, yEdge, orientation, cosAngle, sinAngle, radius;
	int i,k,counter;

	const int width = this->n_cols;
	const int height = this->n_rows;
	const int depth = this->n_slices;

	radius = 0.;
	for ( k=0 ; k<depth ; ++k ) {
		const arma::Mat<T> &currentSlice = this->slice(k);
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).x:width/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).y:height/2;

		orientation = 0.;
		counter = 0;
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
			polygonPoints[counter++] = xEdge;
			polygonPoints[counter++] = yEdge;
		}
		polygonPoints[counter++] = polygonPoints[0];
		polygonPoints[counter] = polygonPoints[1];

		polygon.setPoints(nbPolygonPoints+1,polygonPoints);

		radius += 0.5*qSqrt(polygon.boundingRect().width()*polygon.boundingRect().width() + polygon.boundingRect().height()*polygon.boundingRect().height());
	}

	radius/=depth;
	qDebug() << "Rayon de la boite englobante : " << radius << " (" << radius*_voxelWidth << " mm)";
	return radius*_voxelWidth;
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
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).x:width/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).y:height/2;

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
			radius += qSqrt( xEdge*xEdge + yEdge*yEdge );
		}
	}

	radius/=(depth*nbPolygonPoints);
	qDebug() << "Rayon de la boite englobante (en pixels) : " << radius;
	return radius;
}


#endif // BILLON_H
