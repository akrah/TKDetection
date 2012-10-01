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
#define PI_ON_EIGHT			(PI/8.)
#define THREE_PI_ON_TWO		(3.*PI_ON_TWO)
#define THREE_PI_ON_FOUR	(3.*PI_ON_FOUR)
#define TWO_PI_ON_THREE		(2.*PI_ON_THREE)
#define SEVEN_PI_ON_EIGHT	(7.*PI_ON_EIGHT)
#define DEG_TO_RAD_FACT		(PI/180.)
#define RAD_TO_DEG_FACT		(180./PI)

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

	template<typename T>
	inline T RESTRICT_TO( const T min, const T value, const T max ) {
		return value<min||value>max?min:value;
	}

	inline int MOD ( int x , int y )
	{
		return x >= 0 ? x % y : y - 1 - ((-x-1) % y) ;
	}
}

#define MINIMUM_INTENSITY -900
#define MAXIMUM_INTENSITY 530

#define MINIMUM_Z_MOTION 200
#define MAXIMUM_Z_MOTION 500


template<typename T> struct coord2d;
template<typename T> struct vec2d;

template<typename T>
struct coord2d
{
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d(T x, T y) : x(x), y(y) {}
	inline qreal euclideanDistance( const coord2d &other ) const { return qSqrt( qPow(x-other.x,2) + qPow(y-other.y,2) ); }
	bool operator ==( const coord2d<T> & other ) { return other.x == x && other.y == y; }
	bool operator !=( const coord2d<T> & other ) { return other.x != x || other.y != y; }
	void print( std::ostream &flux ) const { return flux << "( " << x << ", " << y << " )"; }
	qreal angle( const coord2d &bottomCoord, const coord2d &topCoord ) const
	{
		const vec2d<T> AB(bottomCoord.x-x,bottomCoord.y-y);
		const vec2d<T> AC(topCoord.x-x,topCoord.y-y);
		// cos (ABC) = ( prodScalaire(AB,AC) ) / ( norme(AB) * norme (AC) )
		const qreal cosBAC = ( AB.dotProduct(AC) ) / ( AB.norm() * AC.norm() );
		const qreal sinBAC = ( AB.x * AC.y - AB.y * AC.x );
		return sinBAC>0?acos(cosBAC):-acos(cosBAC);
	}
};

template<typename T>
struct vec2d : public coord2d<T>
{
	vec2d() : coord2d<T>((T)0,(T)0) {}
	vec2d( T dx, T dy ) : coord2d<T>(dx,dy) {}
	inline qreal dotProduct( const vec2d<T> & other ) const { return this->x*other.x + this->y*other.y; }
	inline qreal norm() const { return qSqrt( qPow(this->x,2) + qPow(this->y,2) ); }
	// Angle orienté compris entre -180 et 180
	qreal angle( const vec2d<T> &other ) const
	{
		coord2d<T> abscisse(x,y+1);
		qreal distAB = this->euclideanDistance(abscisse);
		qreal distAC = this->euclideanDistance(other);
		return acos( (qPow(distAB, 2.) + qPow(distAC, 2.) - qPow( abscisse.euclideanDistance(other), 2.)) / (2*distAB*distAC) );
	}
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
