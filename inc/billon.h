#ifndef BILLON_H
#define BILLON_H

#include "pith.h"
#include "define.h"
#include "coordinate.h"

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
	Slice& previousSlice( const uint currentSlice );
	const Slice& previousSlice( const uint currentSlice) const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

protected:
	T _minValue;        // Valeur minimum à considérer (mais une valeur PEUT être plus petite car pas de vérification à l'affectation)
	T _maxValue;        // Valeur maximum à considérer (mais une valeur PEUT être plus grande car pas de vérification à l'affectation)
	qreal _voxelWidth;  // Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;  // Profondeur d'un voxel en cm
};

/*######################################################
  # IMPLEMENTATION
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

template< typename T >
Slice& BillonTpl<T>::previousSlice( const uint currentSlice )
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
}

template< typename T >
const Slice& BillonTpl<T>::previousSlice( const uint currentSlice) const
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
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

#endif // BILLON_H
