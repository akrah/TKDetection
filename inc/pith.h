#ifndef MARROW_H
#define MARROW_H

#include <QList>
#include "interval.h"
#include "def/def_coordinate.h"

class QImage;

class Pith : public QList<iCoord2D>
{
public:
	Pith();
	Pith( const int &begin, const int &end );
	Pith( const Pith &pith );

	const Interval<int> &interval() const;

	void draw( QImage &image, const int &sliceIdx ) const;

private:
	Interval<int> _interval;
};

#endif // MARROW_H
