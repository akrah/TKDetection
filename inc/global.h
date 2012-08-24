#ifndef GLOBAL_H
#define GLOBAL_H

#include <qmath.h>
#include <QtGlobal>
#include <ostream>
#include <iostream>

#define PI					(4.*atan(1.0))
#define TWO_PI				(2.*PI)
#define PI_ON_TWO			(PI/2.)
#define PI_ON_THREE			(PI/3.)
#define PI_ON_FOUR			(PI/4.)
#define PI_ON_HEIGHT		(PI/8.)
#define THREE_PI_ON_TWO		(3.*PI_ON_TWO)
#define THREE_PI_ON_FOUR	(3.*PI_ON_FOUR)
#define TWO_PI_ON_THREE		(2.*PI_ON_THREE)
#define DEG_TO_RAD_FACT		(PI/180.)
#define RAD_TO_DEG_FACT		(180./PI)

#define GAUSSIAN_CONSTANT (1./(qSqrt(TWO_PI)))

#define DOUBLE_ERR_POS 0.0000000001
#define DOUBLE_ERR_NEG (-DOUBLE_ERR_POS)

namespace {
	inline qreal ANGLE( const qreal &xo, const qreal &yo, const qreal &x2, const qreal &y2 ) {
		const qreal x_diff = x2-xo;
		const qreal sqrt = qSqrt(qPow(x_diff,2)+pow(y2-yo,2));
		qreal arcos = !qFuzzyCompare(sqrt,0.)?qAcos(x_diff / sqrt):0;
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}

	inline qreal GAUSSIAN( qreal x ) {
		return GAUSSIAN_CONSTANT*qExp(-0.5*(x*x));
	}

	template<typename T>
	inline T RESTRICT_TO( const T min, const T value, const T max ) {
		return value<min||value>max?min:value;
	}
}

#define MINIMUM_INTENSITY -900
#define MAXIMUM_INTENSITY 530

#define MINIMUM_Z_MOTION 200
#define MAXIMUM_Z_MOTION 500


template<typename T>
struct coord2d {
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d(const T &x, const T &y) : x(x), y(y) {}
	coord2d shiftTo( const coord2d &other ) const { return coord2d(other.x-x, other.y-y); }
	qreal distance( const coord2d &other ) const { return qSqrt( qPow(x-other.x,2) + qPow(y-other.y,2) ); }
	qreal angle( const coord2d &other ) const
	{
		coord2d abscisse(x,y+1);
		qreal distAB = this->distance(abscisse);
		qreal distAC = this->distance(other);
		return acos( (qPow(distAB, 2.) + qPow(distAC, 2.) - qPow( abscisse.distance(other), 2.)) / (2*distAB*distAC) );
	}
	qreal angle( const coord2d &bottomCoord, const coord2d &topCoord ) const
	{
		// Vector AB
		const coord2d AB(bottomCoord.x-x,bottomCoord.y-y);
		// Vector AC
		const coord2d AC(topCoord.x-x,topCoord.y-y);
		// cos (ABC) = ( prodScalaire(AB,AC) ) / ( norme(AB) * norme (AC) )
		const qreal cosBAC = ( AB.x * AC.x + AB.y*AC.y ) / ( qSqrt( qPow(AB.x,2) + qPow(AB.y,2) ) * qSqrt( qPow(AC.x,2) + qPow(AC.y,2) ) );
		// signe (BAC) = prodVectoriel(AB,AC)
		const qreal sinBAC = ( AB.x * AC.y - AB.y*AC.x );
		//std::cout << "cos = " << cosBAC << " et signe = " << sinBAC/qAbs(sinBAC) << " ==> angle = " << (sinBAC>0?acos(cosBAC):-acos(cosBAC))*RAD_TO_DEG_FACT << std::endl;
		return sinBAC>0?acos(cosBAC):-acos(cosBAC);
	}
	void print(std::ostream &flux) const { return flux << "( " << x << ", " << y << " )"; }
	bool operator ==( const coord2d<T> &other ) { return other.x == x && other.y == y; }
	bool operator !=( const coord2d<T> &other ) { return other.x != x || other.y != y; }
};

template <typename T>
std::ostream & operator<< ( std::ostream &flux , const coord2d<T> & coord ) {
	coord.print(flux);
	return flux;
}

template<typename T>
struct coord3d {
	T x, y, z;
	coord3d() : x((T)0), y((T)0), z((T)0) {}
	coord3d(const T &x, const T &y, const T &z) : x(x), y(y), z(z) {}
	coord3d shiftTo( const coord3d &other ) const { return coord3d(other.x-x, other.y-y, other.z-z); }
	void print(std::ostream &flux) const { return flux << "( " << x << ", " << y << ", " << z << " )"; }
};

template <typename T>
std::ostream & operator<< ( std::ostream &flux , const coord3d<T> & coord ) {
	coord.print(flux);
	return flux;
}

typedef struct coord2d<int> iCoord2D;
typedef struct coord2d<qreal> rCoord2D;

typedef struct coord3d<int> iCoord3D;
typedef struct coord3d<qreal> rCoord3D;

#endif // GLOBAL_H
