#ifndef PIEPART_H
#define PIEPART_H

#include <QtGlobal>

class PiePart
{
public:
	PiePart();
	PiePart( const qreal &orientation, const qreal &angle );

	qreal angle() const;
	qreal orientation() const;
	qreal rightAngle() const;
	qreal leftAngle() const;

	bool contains( const qreal &angle ) const;

	void setAngle( const qreal &angle );
	void setOrientation( const qreal &orientation );

private:
	qreal _orientation;
	qreal _angle;
	qreal _leftAngle;
	qreal _rightAngle;
};

#endif // PIEPART_H
