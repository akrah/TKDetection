#ifndef PIECHART_DEF_H
#define PIECHART_DEF_H

#include <cmath>

#define PI				(4*atan(1.0))
#define TWO_PI			(2*PI)
#define PI_ON_TWO		(PI/2.)
#define THREE_PI_ON_TWO	(3.*PI_ON_TWO)
#define DEG_TO_RAD_FACT	(PI/180.)

namespace {
	inline double ANGLE( double xo, double yo, double x2, double y2 ) {
		const double x_diff = x2-xo;
		double arcos = std::acos(x_diff / sqrt(pow(x_diff,2)+pow(y2-yo,2)));
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}
}

#endif // PIECHART_DEF_H
