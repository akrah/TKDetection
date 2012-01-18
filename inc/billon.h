#ifndef BILLON_H
#define BILLON_H

#include <armadillo>

using namespace arma;

class Billon : public icube
{
public:
	Billon();
	Billon( const int &width, const int &height, const int &depth );

	int minValue() const;
	int maxValue() const;

	void setMinValue( const int &value );
	void setMaxValue( const int &value );

private:
	int _minValue;
	int _maxValue;
};

#endif // BILLON_H
