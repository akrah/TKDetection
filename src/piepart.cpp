#include "inc/piepart.h"

#include "inc/pie_def.h"

PiePart::PiePart() : _orientation(0.), _angle(TWO_PI) {
	setAngle(_angle);
}

PiePart::PiePart( const qreal &orientation, const qreal &angle ) : _orientation(orientation), _angle(angle) {
	setAngle(angle);
}

/*******************************
 * Public getters
 *******************************/

qreal PiePart::angle() const {
	return _angle;
}

qreal PiePart::orientation() const {
	return _orientation;
}

qreal PiePart::rightAngle() const {
	return _rightAngle;
}

qreal PiePart::leftAngle() const {
	return _leftAngle;
}

bool PiePart::contains( const qreal &angle ) const {
	bool contains = ( (_rightAngle > _leftAngle) && ( angle<_leftAngle || angle>=_rightAngle ) ) || ( (_rightAngle < _leftAngle) && ( angle<_leftAngle && angle>=_rightAngle ) );
	return contains;
}

/*******************************
 * Public setters
 *******************************/

void PiePart::setAngle( const qreal &angle ) {
	_angle = angle;
	_leftAngle = fmod(fmod(_orientation,TWO_PI)+0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
}

void PiePart::setOrientation( const qreal &orientation ) {
	_orientation = orientation;
	_leftAngle = fmod(fmod(_orientation,TWO_PI)+0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
}
