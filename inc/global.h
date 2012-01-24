#ifndef GLOBAL_H
#define GLOBAL_H

#include <qmath.h>

#define PI				(4*atan(1.0))
#define TWO_PI			(2*PI)
#define PI_ON_TWO		(PI/2.)
#define THREE_PI_ON_TWO	(3.*PI_ON_TWO)
#define DEG_TO_RAD_FACT	(PI/180.)

#define DOUBLE_ERR_POS 0.0000000001
#define DOUBLE_ERR_NEG (-DOUBLE_ERR_POS)

namespace {
	inline qreal ANGLE( const qreal &xo, const qreal &yo, const qreal &x2, const qreal &y2 ) {
		const qreal x_diff = x2-xo;
		qreal arcos = qAcos(x_diff / qSqrt(qPow(x_diff,2)+pow(y2-yo,2)));
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}

	inline bool IS_EQUAL( const qreal & v1, const qreal &v2 ) {
		const qreal diff = v1 - v2;
		return (diff < DOUBLE_ERR_POS) && (diff > DOUBLE_ERR_NEG);
	}

	template<class T>
	inline T RESTRICT_TO_INTERVAL(T x, T min, T max) {
		return qMax(min,qMin(max,x));
	}
}

#endif // GLOBAL_H
