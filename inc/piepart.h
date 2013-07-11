#ifndef PIEPART_H
#define PIEPART_H

#include <QtGlobal>

class PiePart
{
public:
	PiePart();
	PiePart( const qreal &orientation, const qreal &angle );
	PiePart( const PiePart &piePart );

	qreal angle() const;
	qreal orientation() const;
	qreal minAngle() const;
	qreal maxAngle() const;

	bool contains( qreal angle ) const;

	void setAngle( const qreal &angle );
	void setOrientation( const qreal &orientation );

private:
	// <En radians>
	qreal _orientation;
	qreal _angle;
	qreal _maxAngle;
	qreal _minAngle;
	// </En radians>
};

#endif // PIEPART_H
