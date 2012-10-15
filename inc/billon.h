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

	const Pith & pith() const;
	Pith &pith();
	bool hasPith() const;
	const iCoord2D &pithCoord( const uint & sliceIndex ) const;
	iCoord2D &pithCoord( const uint & sliceIndex );
	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;
	Slice& previousSlice( const uint currentSlice );
	const Slice& previousSlice( const uint currentSlice) const;
	T zMotion( const uint j, const uint i, const uint sliceInex ) const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );
	void computePith();

protected:
	Pith _pith;

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

template< typename T > BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _pith(0), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}
template< typename T > BillonTpl<T>::BillonTpl( const int & width, const int & height, const int & depth ) : arma::Cube<T>(height,width,depth), _pith(0), _minValue(T(0)), _maxValue(T(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}
template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl<T> & billon ) : arma::Cube<T>(billon), _pith(billon._pith), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}
template< typename T > BillonTpl<T>::BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval ) : arma::Cube<T>(billon.slices(sliceInterval.min(),sliceInterval.max())), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth)
{
	if ( billon.hasPith() ) _pith = billon._pith.mid(sliceInterval.min(),sliceInterval.width()+1);
}

/**********************************
 * Public getters
 **********************************/
template< typename T >
const Pith & BillonTpl<T>::pith() const
{
	return _pith;
}

template< typename T >
Pith &BillonTpl<T>::pith()
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

template< typename T >
iCoord2D &BillonTpl<T>::pithCoord( const uint & sliceIndex )
{
	return _pith[sliceIndex];
}

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

template< typename T >
T BillonTpl<T>::zMotion( const uint j, const uint i, const uint sliceInex ) const
{
	return qAbs( this->at(j,i,sliceInex) - previousSlice(sliceInex).at(j,i) );
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
void BillonTpl<T>::computePith()
{

}

#endif // BILLON_H
