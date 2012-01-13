#ifndef PIEPART_H
#define PIEPART_H

class PiePart
{
public:
	PiePart();
	PiePart( double orientation, double angle );

	double angle() const;
	double orientation() const;
	double rightAngle() const;
	double leftAngle() const;

	bool contains( double angle ) const;

	void setAngle( double angle );
	void setOrientation( double orientation );

private:
	double _orientation;
	double _angle;
	double _leftAngle;
	double _rightAngle;
};

#endif // PIEPART_H
