#ifndef PITHPROFILE_H
#define PITHPROFILE_H

#include "def/def_coordinate.h"
#include "inc/histogram.h"

class Pith;

class PithProfile : public Histogram<qreal>
{
public:
	PithProfile();
	virtual ~PithProfile();

	void construct(const Pith &pith, const uint &smoothingRadius );
};

#endif // PITHPROFILE_H
