#ifndef MARROW_H
#define MARROW_H

#include <QList>
#include "marrow_def.h"
#include "interval.h"

class QImage;

class Marrow : public QList<iCoord2D>
{
public:
	Marrow();
	Marrow( const int &begin, const int &end );

	const Interval &interval() const;

	void draw( QImage &image, const int &sliceIdx ) const;

private:
	Interval _interval;
};

#endif // MARROW_H
