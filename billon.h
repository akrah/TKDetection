#ifndef IMAGE3D_H
#define IMAGE3D_H

#include <armadillo>

using namespace arma;

class Billon : public icube
{
public:
	Billon();
	Billon(int,int,int);

	int minValue() const;
	int maxValue() const;

	void setMinValue(const int);
	void setMaxValue(const int);

private:
	int _minValue;
	int _maxValue;
};

#endif // IMAGE3D_H
