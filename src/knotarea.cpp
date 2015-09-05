#include "inc/knotarea.h"

KnotArea::KnotArea() : _sliceInterval(0,0), _sectorInterval(0,0)
{
}

KnotArea::KnotArea( const Interval<uint> &sliceInterval, const Interval<uint> &sectorInterval ) :
	_sliceInterval(sliceInterval), _sectorInterval(sectorInterval)
{
}

KnotArea::KnotArea( const uint &firstSlice, const uint &firstSector, const uint &nbSlices, const uint &nbSectors ) :
	_sliceInterval( firstSlice, firstSlice+nbSlices ), _sectorInterval( firstSector, firstSector+nbSectors )
{
}

KnotArea::~KnotArea() {}

KnotArea KnotArea::fromQRect( const QRect &rect )
{
	return KnotArea( rect.left(), rect.top(), rect.width(), rect.height() );
}

QRect KnotArea::toQRect() const
{
	return QRect( _sliceInterval.min(), _sectorInterval.min(), _sliceInterval.width(), _sectorInterval.width() );
}

const Interval<uint> &KnotArea::sliceInterval() const
{
	return _sliceInterval;
}

const Interval<uint> &KnotArea::sectorInterval() const
{
	return _sectorInterval;
}

const uint & KnotArea::firstSlice() const
{
	return _sliceInterval.min();
}

const uint & KnotArea::lastSlice() const
{
	return _sliceInterval.max();
}

const uint & KnotArea::firstSector() const
{
	return _sectorInterval.min();
}

const uint & KnotArea::lastSector() const
{
	return _sectorInterval.max();
}

uint KnotArea::nbSlices() const
{
	return _sliceInterval.count();
}

uint KnotArea::nbSectors() const
{
	return _sectorInterval.count();
}

void KnotArea::setSliceBounds( const uint &firstSlice, const uint &lastSlice )
{
	Q_ASSERT_X( firstSlice<lastSlice,
				"void setSliceBounds( const uint &firstSlice, const uint &lastSlice )",
				"L'index de la première coupe doit être inférieur à l'index de la dernière coupe.");
	_sliceInterval.setBounds( firstSlice, lastSlice );
}

void KnotArea::setSliceBoundsFromSize( const uint &firstSlice, const uint &nbSlices )
{
	_sliceInterval.setBounds( firstSlice, firstSlice+nbSlices );
}

void KnotArea::setSectorBounds( const uint &firstSector, const uint &lastSector )
{
	Q_ASSERT_X( firstSector<lastSector,
				"void setSectorBounds( const uint &firstSector, const uint &lastSector )",
				"L'index du premier secteur doit être inférieur à l'index du dernioer secteur.");
	_sectorInterval.setBounds( firstSector, lastSector );
}

void KnotArea::setSectorBoundsFromSize( const uint &firstSector, const uint &nbSectors )
{
	_sectorInterval.setBounds( firstSector, firstSector+nbSectors );
}

void KnotArea::setBounds( const uint &firstSlice, const uint &firstSector, const uint &lastSlice, const uint &lastSector )
{
	setSliceBounds( firstSlice, lastSlice );
	setSectorBounds( firstSector, lastSector );
}

void  KnotArea::setBoundsFromSize( const uint &firstSlice, const uint &firstSector, const uint &nbSlices, const uint &nbSectors )
{
	setBounds( firstSlice, firstSector, firstSlice+nbSlices, firstSector+nbSectors );
}
