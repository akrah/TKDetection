#include "inc/piepart.h"

#include "inc/define.h"

#include <cmath>

PiePart::PiePart() : _orientation(0.), _angle(0.)
{
	setAngle(_angle);
}

PiePart::PiePart( const qreal &orientation, const qreal &angle ) : _orientation(orientation), _angle(angle)
{
	setAngle(angle);
}

PiePart::PiePart( const PiePart &piePart ) : _orientation(piePart._orientation), _angle(piePart._angle), _leftAngle(piePart._leftAngle), _rightAngle(piePart._rightAngle)
{
}

/*******************************
 * Public getters
 *******************************/

qreal PiePart::angle() const
{
	return _angle;
}

qreal PiePart::orientation() const
{
	return _orientation;
}

qreal PiePart::rightAngle() const
{
	return _rightAngle;
}

qreal PiePart::leftAngle() const
{
	return _leftAngle;
}

bool PiePart::contains( const qreal &angle ) const
{
	bool contains = _rightAngle < _leftAngle ? angle<_leftAngle && angle>=_rightAngle : angle<_leftAngle || angle>=_rightAngle;
	return contains;
}

/*******************************
 * Public setters
 *******************************/

void PiePart::setAngle( const qreal &angle )
{
	_angle = angle;
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_leftAngle = fmod(_rightAngle+angle,TWO_PI);
}

void PiePart::setOrientation( const qreal &orientation )
{
	_orientation = orientation;
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_leftAngle = fmod(_rightAngle+angle(),TWO_PI);
}
