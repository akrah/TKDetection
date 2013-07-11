#include "inc/piepart.h"

#include "inc/define.h"

#include <cmath>

PiePart::PiePart() : _orientation(0.), _angle(0.)
{
	setAngle(0.);
}

PiePart::PiePart( const qreal &orientation, const qreal &angle ) : _orientation(fmod(orientation+TWO_PI,TWO_PI)), _angle(angle)
{
	setAngle(angle);
}

PiePart::PiePart( const PiePart &piePart ) : _orientation(piePart._orientation), _angle(piePart._angle), _maxAngle(piePart._maxAngle), _minAngle(piePart._minAngle)
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

qreal PiePart::minAngle() const
{
	return _minAngle;
}

qreal PiePart::maxAngle() const
{
	return _maxAngle;
}

bool PiePart::contains( qreal angle ) const
{
	angle = fmod(angle+TWO_PI,TWO_PI);
	bool contains = _minAngle < _maxAngle ? angle<_maxAngle && angle>=_minAngle : angle<_maxAngle || angle>=_minAngle;
	return contains;
}

/*******************************
 * Public setters
 *******************************/

void PiePart::setAngle( const qreal &angle )
{
	_angle = angle;
	_minAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_maxAngle = fmod(_minAngle+angle,TWO_PI);
}

void PiePart::setOrientation( const qreal &orientation )
{
	_orientation = orientation;
	_minAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_maxAngle = fmod(_minAngle+angle(),TWO_PI);
}
