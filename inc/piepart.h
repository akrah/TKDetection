#ifndef PIEPART_H
#define PIEPART_H

class PiePart
{
public:
	PiePart();
	PiePart( const double &orientation, const double &angle );

	double angle() const;
	double orientation() const;
	double rightAngle() const;
	double leftAngle() const;

	bool contains( const double &angle ) const;

	void setAngle( const double &angle );
	void setOrientation( const double &orientation );

private:
	double _orientation;
	double _angle;
	double _leftAngle;
	double _rightAngle;
};

#endif // PIEPART_H
