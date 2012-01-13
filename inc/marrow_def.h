#ifndef MARROW_DEF_H
#define MARROW_DEF_H

#include <ostream>

typedef struct coord2d {
	int x, y;
	coord2d() : x(0), y(0) {}
	coord2d(int x, int y) : x(x), y(y) {}
} Coord2D;

typedef struct coord3d {
	int x, y, z;
	coord3d() : x(0), y(0), z(0) {}
	coord3d(int x, int y, int z) : x(x), y(y), z(z) {}
} Coord3D;

std::ostream& operator<<( std::ostream &flux, Coord2D const& coord );

#endif // MARROW_DEF_H
