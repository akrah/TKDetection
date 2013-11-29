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

void KnotPithProfile::construct( const Pith &pith, const rCoord3D &voxelDims, const uint &smoothingRadius )
{
	int size = pith.size();
	clear();

	if (!size) return;

	//const qreal &voxelRatio = voxelDims.y/(4.*voxelDims.z);
	const qreal &voxelRatio = 0.25;

	resize(size);
	for ( int k=2 ; k<size-2 ; ++k )
	{
		(*this)[k] = qAtan((pith[k-1].y-pith[k+1].y)*voxelRatio); //à ajouter si on veut prendre ne compte les mm
	}
	(*this)[0] = (*this)[1] = (*this)[2];
	(*this)[size-1] = (*this)[size-2] = (*this)[size-3];

	if (smoothingRadius) TKD::meanSmoothing<qreal>( this->begin(), this->end(), smoothingRadius );
}
