#ifndef BILLON_H
#define BILLON_H

#include "global.h"
#include "marrow.h"

#include <qglobal.h>
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

	BillonTpl * restrictToArea( const int &nbPolygonsPoints, const int &threshold, const Marrow *marrow = 0 );

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
BillonTpl<T> * BillonTpl<T>::restrictToArea( const int &nbPolygonPoints, const int &threshold, const Marrow *marrow ) {
	BillonTpl *restrictedBillon = new BillonTpl(*this);
	restrictedBillon->setMinValue(threshold);
	const int nbSlices = restrictedBillon->n_slices;
	const int thresholdRestrict = threshold-1;

	for ( int indexSlice = 0 ; indexSlice<nbSlices ; ++indexSlice ) {
		arma::Mat<T> &currentSlice = restrictedBillon->slice(indexSlice);

		const int sliceWidth = currentSlice.n_cols;
		const int sliceHeight = currentSlice.n_rows;
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).x:sliceWidth/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).y:sliceHeight/2;

		QPolygon polygon(nbPolygonPoints);
		int polygonPoints[2*nbPolygonPoints+2];

		qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
		int i,j,k;
		orientation = 0.;
		k = 0;

		for ( i=0 ; i<nbPolygonPoints ; ++i ) {
			orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			cosAngle = qCos(orientation);
			sinAngle = -qSin(orientation);
			xEdge = xCenter + 5*cosAngle;
			yEdge = yCenter + 5*sinAngle;
			while ( xEdge>0. && yEdge>0. && xEdge<sliceWidth && yEdge<sliceHeight && currentSlice.at(yEdge,xEdge) > thresholdRestrict ) {
				xEdge += cosAngle;
				yEdge += sinAngle;
			}
			polygonPoints[k++] = xEdge;
			polygonPoints[k++] = yEdge;
		}
		polygonPoints[k++] = polygonPoints[0];
		polygonPoints[k] = polygonPoints[1];

		polygon.setPoints(nbPolygonPoints+1,polygonPoints);

		QRect boudingRect = polygon.boundingRect();
		const int xLeft = qMax(0,boudingRect.left());
		const int xRight = qMin(sliceWidth-1,boudingRect.right());
		const int yTop = qMax(0,boudingRect.top());
		const int yBottom = qMin(sliceHeight-1,boudingRect.bottom()+1);

		currentSlice.submat(0,0,yTop,sliceWidth-1).fill(threshold);
		currentSlice.submat(yTop,0,yBottom,xLeft).fill(threshold);
		currentSlice.submat(yTop,xRight,yBottom,sliceWidth-1).fill(threshold);
		currentSlice.submat(yBottom,0,sliceHeight-1,sliceWidth-1).fill(threshold);

		polygon.translate(-xLeft,-yTop);
		arma::Mat<T> boudingSlice = currentSlice.submat( arma::span(yTop,yBottom), arma::span(xLeft,xRight) );
		const int boudingWidth = boudingSlice.n_cols;
		const int boudinHeight = boudingSlice.n_rows;
		for ( j=0 ; j<boudinHeight ; ++j ) {
			for ( i=0 ; i<boudingWidth ; ++i ) {
				if ( !polygon.containsPoint(QPoint(i,j),Qt::WindingFill) ) {
					boudingSlice.at(j,i) = threshold;
				}
			}
		}
		currentSlice( arma::span(yTop,yBottom), arma::span(xLeft,xRight) ) = boudingSlice;
	}

	return restrictedBillon;
}

#endif // BILLON_H
