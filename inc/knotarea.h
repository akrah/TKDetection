#ifndef KNOTAREA_H
#define KNOTAREA_H

#include "inc/interval.h"

#include <QRect>

class KnotArea
{
public:
	KnotArea();
	KnotArea( const Interval<uint> &sliceInterval, const Interval<uint> &sectorInterval );
	KnotArea( const uint &firstSlice, const uint &firstSector, const uint &nbSlices, const uint &nbSectors );
	~KnotArea();

	static KnotArea fromQRect( const QRect &rect );
	QRect toQRect() const;

	const Interval<uint> &sliceInterval() const;
	const Interval<uint> &sectorInterval() const;

	const uint & firstSlice() const;
	const uint & lastSlice() const;
	const uint & firstSector() const;
	const uint & lastSector() const;

	uint nbSlices() const;
	uint nbSectors() const;

	void setSliceBounds( const uint &firstSlice, const uint &lastSlice );
	void setSliceBoundsFromSize( const uint &firstSlice, const uint &nbSlices );
	void setSectorBounds( const uint &firstSector, const uint &lastSector );
	void setSectorBoundsFromSize( const uint &firstSector, const uint &nbSectors );
	void setBounds(const uint &firstSlice, const uint &firstSector, const uint &lastSlice, const uint &lastSector );
	void setBoundsFromSize( const uint &firstSlice, const uint &firstSector, const uint &nbSlices, const uint &nbSectors );

private:

private:
	Interval<uint> _sliceInterval;
	Interval<uint> _sectorInterval; // Cet interval est toujours valide (min<max)
};

#endif // KNOTAREA_H
