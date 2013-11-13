#ifndef KNOTPITHPROFILE_H
#define KNOTPITHPROFILE_H

#include "inc/histogram.h"

class Pith;

class KnotPithProfile : public Histogram<qreal>
{
public:
	KnotPithProfile();
	virtual ~KnotPithProfile();

	void construct( const Pith &pith );
};

#endif // KNOTPITHPROFILE_H
