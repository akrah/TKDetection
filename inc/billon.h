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

	void setMinValue( const T &value );
	void setMaxValue( const T &value );

protected:
	T _minValue;
	T _maxValue;
};

template< typename T >
BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _minValue((T)0), _maxValue((T)0) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int &width, const int &height, const int &depth ) : arma::Cube<T>(height,width,depth), _minValue((T)0), _maxValue((T)0) {}

template< typename T >
T BillonTpl<T>::minValue() const {
	return _minValue;
}

template< typename T >
T BillonTpl<T>::maxValue() const {
	return _maxValue;
}

template< typename T >
void BillonTpl<T>::setMinValue( const T &value ) {
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T &value ) {
	_maxValue = value;
}

#endif // BILLON_H
