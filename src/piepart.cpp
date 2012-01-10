#include "inc/piepart.h"

PiePart::PiePart() : _orientation(0.), _angle(TWO_PI) {
	setAngle(_angle);
}

PiePart::PiePart( double orientation, double angle ) : _orientation(orientation), _angle(angle) {
	setAngle(angle);
}

double PiePart::angle() const {
	return _angle;
}

double PiePart::orientation() const {
	return _orientation;
}

double PiePart::rightAngle() const {
	return _rightAngle;
}

double PiePart::leftAngle() const {
	return _leftAngle;
}

bool PiePart::contains( double angle ) const {
	bool contains = ( (_rightAngle > _leftAngle) && ( angle<_leftAngle || angle>=_rightAngle ) ) || ( (_rightAngle < _leftAngle) && ( angle<_leftAngle && angle>=_rightAngle ) );
	return contains;
}

void PiePart::setAngle( double angle ) {
	_angle = angle;
	_leftAngle = fmod(fmod(_orientation,TWO_PI)+0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
}

void PiePart::setOrientation( double orientation ) {
	_orientation = orientation;
	_leftAngle = fmod(fmod(_orientation,TWO_PI)+0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
	_rightAngle = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
}
