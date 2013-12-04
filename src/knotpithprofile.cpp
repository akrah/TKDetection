#include "inc/knotpithprofile.h"

#include "inc/pith.h"
#include "inc/coordinate.h"

#include "inc/globalfunctions.h"

KnotPithProfile::KnotPithProfile() : Histogram<qreal>()
{
}

KnotPithProfile::~KnotPithProfile()
{
}

/**********************************
 * Public setters
 **********************************/

void KnotPithProfile::construct( const Pith &pith, const uint &smoothingRadius )
{
	int size = pith.size();
	resize(size);

	if (!size) return;

	for ( int k=2 ; k<size-2 ; ++k )
	{
		(*this)[k] = qAtan((pith[k-2].y-pith[k+2].y)/4.);
	}
	(*this)[0] = (*this)[1] = (*this)[2];
	(*this)[size-1] = (*this)[size-2] = (*this)[size-3];

	if (smoothingRadius) TKD::meanSmoothing<qreal>( this->begin(), this->end(), smoothingRadius );
}
