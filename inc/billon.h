#ifndef BILLON_H
#define BILLON_H

#include "pith.h"
#include "define.h"
#include "coordinate.h"
#include "interval.h"

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
	friend class PithExtractorBoukadida;

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
	const rCoord3D &voxelDims() const;
	const int &zPos() const;

	const Slice& previousSlice( const uint currentSlice) const;
	Slice& previousSlice( const uint currentSlice );
	const Slice& nextSlice( const uint currentSlice) const;
	Slice& nextSlice( const uint currentSlice );
	T zMotion( const uint i, const uint j, const uint sliceIndex ) const;

	const Pith & pith() const;
	bool hasPith() const;
	const rCoord2D &pithCoord( const uint & sliceIndex ) const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );
	void setZPos( const int &zPos );

protected:
	Pith _pith;

	Interval<T> _valueInterval;
	T _minValue;        // Valeur minimum à considérer (mais une valeur PEUT être plus petite car pas de vérification à l'affectation)
	T _maxValue;        // Valeur maximum à considérer (mais une valeur PEUT être plus grande car pas de vérification à l'affectation)
	rCoord3D _voxelDims;
	qreal _voxelWidth;  // Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;  // Profondeur d'un voxel en cm
	int _zPos;          // Position du billon dans l'espace 3D (en coordonnées entières)
};

/*######################################################
  # IMPLEMENTATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template< typename T >
BillonTpl<T>::BillonTpl() :
	arma::Cube<T>(), _pith(0), _valueInterval(T(0),T(0)), _voxelDims(1.,1.,1.), _zPos(0) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int & width, const int & height, const int & depth ) :
	arma::Cube<T>(height,width,depth), _pith(0), _valueInterval(T(0),T(0)), _voxelDims(1.,1.,1.), _zPos(0) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl<T> & billon ) :
	arma::Cube<T>(billon), _pith(billon._pith), _valueInterval(billon._valueInterval), _voxelDims(billon._voxelDims), _zPos(billon._zPos) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl<T> &billon, const Interval<uint> &sliceInterval ) :
	arma::Cube<T>(billon.slices(sliceInterval.min(),sliceInterval.max())), _valueInterval(billon._valueInterval), _voxelDims(billon._voxelDims), _zPos(billon._zPos+sliceInterval.min())
{
	if ( billon.hasPith() ) _pith = billon._pith.mid(sliceInterval.min(),sliceInterval.width()+1);
}

/**********************************
 * Public getters
 **********************************/

template< typename T >
const T &BillonTpl<T>::minValue() const
{
	return _valueInterval.min();
}

template< typename T >
const T &BillonTpl<T>::maxValue() const
{
	return _valueInterval.max();
}

template< typename T >
const qreal &BillonTpl<T>::voxelWidth() const
{
	return _voxelDims.x;
}

template< typename T >
const qreal &BillonTpl<T>::voxelHeight() const
{
	return _voxelDims.y;
}

template< typename T >
const qreal &BillonTpl<T>::voxelDepth() const
{
	return _voxelDims.z;
}

template< typename T >
const rCoord3D &BillonTpl<T>::voxelDims() const
{
	return _voxelDims;
}

template< typename T >
const int &BillonTpl<T>::zPos() const
{
	return _zPos;
}

template< typename T >
const Slice& BillonTpl<T>::previousSlice( const uint currentSlice ) const
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
}

template< typename T >
Slice& BillonTpl<T>::previousSlice( const uint currentSlice )
{
	return this->slice(currentSlice>0 ? currentSlice-1 : currentSlice+1);
}

template< typename T >
const Slice& BillonTpl<T>::nextSlice( const uint currentSlice ) const
{
	return this->slice(currentSlice<this->n_slices-1 ? currentSlice+1 : currentSlice-1);
}

template< typename T >
Slice& BillonTpl<T>::nextSlice( const uint currentSlice )
{
	return this->slice(currentSlice<this->n_slices-1 ? currentSlice+1 : currentSlice-1);
}

template< typename T >
T BillonTpl<T>::zMotion( const uint i, const uint j, const uint sliceIndex ) const
{
	return qAbs( (*this)(j,i,sliceIndex) - previousSlice(sliceIndex)(j,i) );
	//return qAbs( nextSlice(sliceIndex)(j,i) - previousSlice(sliceIndex)(j,i) );
	//return 0.25 * qAbs( -2 * (*this)(j,i,sliceIndex) + previousSlice(sliceIndex)(j,i) + nextSlice(sliceIndex)(j,i));
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
const rCoord2D &BillonTpl<T>::pithCoord( const uint & sliceIndex ) const
{
	return _pith[sliceIndex];
}

/**********************************
 * Public setters
 **********************************/

template< typename T >
void BillonTpl<T>::setMinValue( const T & value )
{
	_valueInterval.setMin(value);
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T & value )
{
	_valueInterval.setMax(value);
}

template< typename T >
void BillonTpl<T>::setVoxelSize( const qreal & width, const qreal & height, const qreal & depth )
{
	_voxelDims.x = width;
	_voxelDims.y = height;
	_voxelDims.z = depth;
}

template< typename T >
void BillonTpl<T>::setZPos( const int &zPos )
{
	_zPos = zPos;
}

#endif // BILLON_H
