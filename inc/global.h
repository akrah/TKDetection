#ifndef GLOBAL_H
#define GLOBAL_H

#include <qmath.h>

#define PI				(4.*atan(1.0))
#define TWO_PI			(2.*PI)
#define PI_ON_TWO		(PI/2.)
#define PI_ON_FOUR		(PI/4.)
#define THREE_PI_ON_TWO	(3.*PI_ON_TWO)
#define DEG_TO_RAD_FACT	(PI/180.)
#define RAD_TO_DEG_FACT	(180./PI)

#define DOUBLE_ERR_POS 0.0000000001
#define DOUBLE_ERR_NEG (-DOUBLE_ERR_POS)

namespace {
	inline bool IS_EQUAL( const qreal & v1, const qreal &v2 ) {
		const qreal diff = v1 - v2;
		return (diff < DOUBLE_ERR_POS) && (diff > DOUBLE_ERR_NEG);
	}

	inline qreal ANGLE( const qreal &xo, const qreal &yo, const qreal &x2, const qreal &y2 ) {
		const qreal x_diff = x2-xo;
		const qreal sqrt = qSqrt(qPow(x_diff,2)+pow(y2-yo,2));
		qreal arcos = !IS_EQUAL(sqrt,0.)?qAcos(x_diff / sqrt):0;
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}

	template<class T>
	inline T RESTRICT_TO_INTERVAL(const T &x, const T &min, const T &max) {
		return (x<min||x>max)?min:x;
	}
}

#endif // GLOBAL_H
