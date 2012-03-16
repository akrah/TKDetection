#ifndef MARROW_DEF_H
#define MARROW_DEF_H

#include <QtGlobal>
#include <ostream>

template<typename T>
struct coord2d {
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d(const T &x, const T &y) : x(x), y(y) {}
	coord2d shiftTo( const coord2d &other ) const { return coord2d(other.x-x, other.y-y); }
	void print(std::ostream &flux) const { return flux << "( " << x << ", " << y << " )"; }
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
