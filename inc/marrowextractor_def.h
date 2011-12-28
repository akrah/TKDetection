#ifndef MARROWEXTRACTOR_DEF_H
#define MARROWEXTRACTOR_DEF_H

#include <QList>

// Paramètres par défaut pour l'extraction de la moelle
namespace MarrowExtractorDefaultsParameters {
	extern int FALSE_CUT_PERCENT;
	extern int NEIGHBORHOOD_WINDOW_WIDTH;
	extern int NEIGHBORHOOD_WINDOW_HEIGHT;
	extern int BINARIZATION_THRESHOLD;
	extern int MARROW_LAG;
}

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

typedef struct marrow {
	int begin;
	int end;
	QList<Coord2D> list;

	marrow() : begin(0),end(0) {}
	marrow(int begin, int end) : begin(begin), end(end) {}

	int size() { return end-begin; }

	Coord2D& operator[] (uint i) { return list[i]; }
	const Coord2D& operator() (uint i) const { return list.at(i); }
} Marrow;

std::ostream& operator<<( std::ostream &flux, Coord2D const& coord );

#endif // MARROWEXTRACTOR_DEF_H
