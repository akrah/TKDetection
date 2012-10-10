#ifndef DEF_COORDINATE_H
#define DEF_COORDINATE_H

#include <QtGlobal>

// Coordinates and Vect
template<typename T> struct coord2d;
template<typename T> struct vec2d;
template<typename T> struct coord3d;

typedef struct coord2d<int> iCoord2D;
typedef struct coord2d<uint> uiCoord2D;
typedef struct coord2d<qreal> rCoord2D;
typedef struct vec2d<int> iVec2D;
typedef struct vec2d<uint> uiVec2D;
typedef struct vec2d<qreal> rVec2D;
typedef struct coord3d<int> iCoord3D;
typedef struct coord3d<uint> uiCoord3D;
typedef struct coord3d<qreal> rCoord3D;

#endif // DEF_COORDINATE_H
