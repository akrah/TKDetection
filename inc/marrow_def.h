#ifndef MARROW_DEF_H
#define MARROW_DEF_H

#include <QtGlobal>
#include <ostream>
#include <qmath.h>

template<typename T>
struct coord2d {
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d(const T &x, const T &y) : x(x), y(y) {}
	coord2d shiftTo( const coord2d &other ) const { return coord2d(other.x-x, other.y-y); }
	qreal distance( const coord2d &other ) const { return qSqrt( qPow(x-other.x,2) + qPow(y-other.y,2) ); }
	qreal angle( const coord2d &bottomCoord, const coord2d &topCoord ) const
	{
		qreal distAB = this->distance(bottomCoord);
		qreal distAC = this->distance(topCoord);
		return acos( (qPow(distAB, 2.) + qPow(distAC, 2.) - qPow( bottomCoord.distance(topCoord), 2.)) / (2*distAB*distAC) );
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

#endif // MARROW_DEF_H
