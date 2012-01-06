#ifndef PIECHART_H
#define PIECHART_H

#define PI     4*atan(1.0)
#define TWO_PI 2*PI

#include <cmath>

typedef struct {
	double _angle;
	double _orientation;

	double rightFactor() const {
		const double res = fmod(fmod(_orientation,TWO_PI)-0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
		return tan(res);
	}

	double leftFactor() const {
		const double res = fmod(fmod(_orientation,TWO_PI)+0.5*fmod(_angle,TWO_PI)+TWO_PI,TWO_PI);
		return tan(res);
	}
} PiePart;

class PieChart
{
public:
	PieChart();

private:
	double _orientation;
	double _angle;
};

#endif // PIECHART_H
