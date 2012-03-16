#ifndef BILLON_H
#define BILLON_H

#include <qglobal.h>
#include <armadillo>

template< typename T >
class BillonTpl : public arma::Cube<T>
{
public:
	BillonTpl();
	BillonTpl( const int &width, const int &height, const int &depth );

	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

protected:
	T _minValue;
	T _maxValue;
	qreal _voxelWidth;	// Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;	// Profondeur d'un voxel en cm
};

template< typename T >
BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _minValue((T)0), _maxValue((T)0), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int &width, const int &height, const int &depth ) : arma::Cube<T>(height,width,depth), _minValue((T)0), _maxValue((T)0), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

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

#endif // BILLON_H
