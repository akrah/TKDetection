#ifndef KNOTPITHPROFILE_H
#define KNOTPITHPROFILE_H

#include "def/def_coordinate.h"
#include "inc/histogram.h"

class Pith;

class KnotPithProfile : public Histogram<qreal>
{
public:
	KnotPithProfile();
	virtual ~KnotPithProfile();

	void construct( const Pith &pith , const rCoord3D &voxelDims );
};

#endif // KNOTPITHPROFILE_H
