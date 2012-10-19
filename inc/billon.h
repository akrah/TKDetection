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
	friend class PithExtractor;

public:
	BillonTpl();
	BillonTpl( const int & width, const int & height, const int & depth );
	BillonTpl( const BillonTpl<T> &billon );
	BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval );

	const T &minValue() const;
	const T &maxValue() const;
	const qreal &voxelWidth() const;
	const qreal &voxelHeight() const;
	const qreal &voxelDepth() const;
	const int &xPos() const;
	const int &yPos() const;
	const int &zPos() const;

	const Slice& previousSlice( const uint currentSlice) const;
	Slice& previousSlice( const uint currentSlice );
	T zMotion( const uint j, const uint i, const uint sliceInex ) const;
	BillonTpl<T> subBillon( const BillonTpl<T> &billon, const int &firstX, const int &firstY, const int &firstSlice, const int &lastX, const int &lastY, const int &lastSlice ) const;

	const Pith & pith() const;
	bool hasPith() const;
	const iCoord2D &pithCoord( const uint & sliceIndex ) const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );
	void setPos( const iCoord3D &newPos );

protected:
	Pith _pith;

	T _minValue;        // Valeur minimum à considérer (mais une valeur PEUT être plus petite car pas de vérification à l'affectation)
	T _maxValue;        // Valeur maximum à considérer (mais une valeur PEUT être plus grande car pas de vérification à l'affectation)
	qreal _voxelWidth;  // Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;  // Profondeur d'un voxel en cm
	iCoord3D _pos;      // Position du billon dans l'espace 3D (en coordonnées entières)
};

/*######################################################
  # IMPLEMENTATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template< typename T > BillonTpl<T>::BillonTpl() :
	arma::Cube<T>(), _pith(0), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.), _pos(iCoord3D(0,0,0)) {}

template< typename T > BillonTpl<T>::BillonTpl( const int & width, const int & height, const int & depth ) :
	arma::Cube<T>(height,width,depth), _pith(0), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.), _pos(iCoord3D(0,0,0)) {}

template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl<T> & billon ) :
	arma::Cube<T>(billon), _pith(billon._pith), _minValue(billon._minValue), _maxValue(billon._maxValue),
	_voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth), _pos(billon._pos) {}

template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval ) :
	arma::Cube<T>(billon.slices(sliceInterval.min(),sliceInterval.max())), _minValue(billon._minValue), _maxValue(billon._maxValue),
	_voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth), _pos(billon._pos)
{
	if ( billon.hasPith() ) _pith = billon._pith.mid(sliceInterval.min(),sliceInterval.width()+1);
	_pos.z = billon._pos.z+sliceInterval.min();
}

/**********************************
 * Public getters
 **********************************/

template< typename T >
const T &BillonTpl<T>::minValue() const
{
	return _minValue;
}

template< typename T >
const T &BillonTpl<T>::maxValue() const
{
	return _maxValue;
}

template< typename T >
const qreal &BillonTpl<T>::voxelWidth() const
{
	return _voxelWidth;
}

template< typename T >
const qreal &BillonTpl<T>::voxelHeight() const
{
	return _voxelHeight;
}

template< typename T >
const qreal &BillonTpl<T>::voxelDepth() const
{
	return _voxelDepth;
}

template< typename T >
const int &BillonTpl<T>::xPos() const
{
	return  _pos.x;
}

template< typename T >
const int &BillonTpl<T>::yPos() const
{
	return _pos.y;
}

template< typename T >
const int &BillonTpl<T>::zPos() const
{
	return _pos.z;
}

template< typename T >
const Slice& BillonTpl<T>::previousSlice( const uint currentSlice) const
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
}

template< typename T >
Slice& BillonTpl<T>::previousSlice( const uint currentSlice )
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
}

template< typename T >
T BillonTpl<T>::zMotion( const uint j, const uint i, const uint sliceInex ) const
{
	return qAbs( this->at(j,i,sliceInex) - previousSlice(sliceInex).at(j,i) );
}

template< typename T >
BillonTpl<T> BillonTpl<T>::subBillon( const BillonTpl<T> &billon, const int &firstX, const int &firstY, const int &firstSlice, const int &lastX, const int &lastY, const int &lastSlice ) const
{
	BillonTpl<T> sub = billon.subcube( firstY, firstX, firstSlice, lastY, lastX, lastSlice );
	sub._minValue = billon._minValue;
	sub._maxValue = billon._maxValue;
	sub._voxelWidth = billon._voxelWidth;
	sub._voxelHeight = billon._voxelHeight;
	sub._voxelDepth = billon._voxelDepth;
	if ( billon.hasPith() ) _pith = billon._pith.mid(firstSlice,lastSlice-firstSlice+1);
	_pos = billon._pos + iCoord3D(firstX,firstY,firstSlice);
	return sub;
}

template< typename T >
const Pith & BillonTpl<T>::pith() const
{
	return _pith;
}

template< typename T >
bool BillonTpl<T>::hasPith() const
{
	return !_pith.isEmpty();
}

template< typename T >
const iCoord2D &BillonTpl<T>::pithCoord( const uint & sliceIndex ) const
{
	return _pith[sliceIndex];
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
void BillonTpl<T>::setPos( const iCoord3D &newPos )
{
	_pos = newPos;
}

#endif // BILLON_H
