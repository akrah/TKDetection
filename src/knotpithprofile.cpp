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

	if (size<4) return;

	(*this)[0] = qAtan((pith[0].y-pith[2].y)/2.);
	(*this)[1] = qAtan((pith[0].y-pith[3].y)/3.);
	for ( int k=2 ; k<size-2 ; ++k )
	{
		(*this)[k] = qAtan((pith[k-2].y-pith[k+2].y)/4.);
	}
	(*this)[size-2] = qAtan((pith[size-4].y-pith[size-1].y)/3.);
	(*this)[size-1] = qAtan((pith[size-3].y-pith[size-1].y)/2.);

	if (smoothingRadius) TKD::meanSmoothing<qreal>( this->begin(), this->end(), smoothingRadius );

	for ( int k=0 ; k<size ; ++k )
	{
		(*this)[k] = qCos((*this)[k]);
	}
}
