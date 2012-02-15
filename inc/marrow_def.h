#ifndef MARROW_DEF_H
#define MARROW_DEF_H

#include <QtGlobal>
#include <ostream>

typedef struct coord2d {
	int x, y;
	coord2d() : x(0), y(0) {}
	coord2d(const int &x, const int &y) : x(x), y(y) {}
	coord2d shiftTo( const coord2d &other ) const { return coord2d(other.x-x, other.y-y); }
} Coord2D;

typedef struct coord3d {
	int x, y, z;
	coord3d() : x(0), y(0), z(0) {}
	coord3d(const int &x, const int &y, const int &z) : x(x), y(y), z(z) {}
	coord3d shiftTo( const coord3d &other ) const { return coord3d(other.x-x, other.y-y, other.z-z); }
} Coord3D;

std::ostream& operator<<( std::ostream &flux, const Coord2D & coord );

#endif // MARROW_DEF_H
