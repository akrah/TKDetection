#include "inc/piepart.h"

namespace {
	inline double ANGLE( double xo, double yo, double x2, double y2 ) {
		const double x_diff = x2-xo;
		double arcos = std::acos(x_diff / sqrt(pow(x_diff,2)+pow(y2-yo,2)));
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}
}

PiePart::PiePart( double angle, double orientation ) : _angle(angle), _orientation(orientation) {
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
