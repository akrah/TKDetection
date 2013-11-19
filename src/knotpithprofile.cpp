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

void KnotPithProfile::construct( const Pith &pith, const rCoord3D &voxelDims )
{
	int size = pith.size();
	clear();

	if (!size) return;

	const qreal &voxelRatio = voxelDims.y/(4.*voxelDims.z);

	resize(size);
	for ( int k=2 ; k<size-2 ; ++k )
	{
		(*this)[k] = qAtan((pith[k-2].y-pith[k+2].y)*voxelRatio); //à ajouter si on veut prendre ne compte les mm
	}
	(*this)[0] = (*this)[1] = (*this)[2];
	(*this)[size-1] = (*this)[size-2] = (*this)[size-3];
}
