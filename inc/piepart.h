#ifndef PIEPART_H
#define PIEPART_H

#include <cmath>

#define PI      (4*atan(1.0))
#define TWO_PI  (2*PI)

class PiePart
{
public:
	PiePart( double angle, double orientation );

	double angle() const;
	double orientation() const;
	double rightAngle() const;
	double leftAngle() const;

	void setAngle( double angle );
	void setOrientation( double orientation );

private:
	double _angle;
	double _orientation;
	double _leftAngle;
	double _rightAngle;
};

#endif // PIEPART_H
