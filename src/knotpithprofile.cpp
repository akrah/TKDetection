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
	const int &size = pith.size()-1;

	clear();
	resize(size+1);

	for ( int k=1 ; k<size ; ++k )
	{
		(*this)[k] = (pith[k+1].x-pith[k-1].x)/2.;
	}
}
