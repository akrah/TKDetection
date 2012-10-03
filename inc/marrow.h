#ifndef MARROW_H
#define MARROW_H

#include <QList>
#include "interval.h"
#include "def/def_coordinate.h"

class QImage;

class Marrow : public QList<iCoord2D>
{
public:
	Marrow();
	Marrow( const int &begin, const int &end );
	Marrow( const Marrow &marrow );

	const Interval<int> &interval() const;

	void draw( QImage &image, const int &sliceIdx ) const;

private:
	Interval<int> _interval;
};

#endif // MARROW_H
