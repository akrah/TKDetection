#include "inc/knotpithprofile.h"

#include "inc/pith.h"
#include "inc/coordinate.h"

KnotPithProfile::KnotPithProfile() : Histogram<qreal>()
{
}

KnotPithProfile::~KnotPithProfile()
{
}

/**********************************
 * Public setters
 **********************************/

void KnotPithProfile::construct( const Pith &pith )
{
	int size = pith.size();
	clear();

	if (!size) return;

	resize(size);

	size--;
	for ( int k=1 ; k<size ; ++k )
	{
		(*this)[k] = qAtan((pith[k+1].x-pith[k-1].x)/2.);
	}
	(*this)[0] = (*this)[1];
	(*this)[size] = (*this)[size-1];
}
