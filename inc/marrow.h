#ifndef MARROW_H
#define MARROW_H

#include <QList>
#include "marrow_def.h"

class QPainter;

class Marrow : public QList<Coord2D>
{
public:
	Marrow();
	Marrow( int begin, int end );

	int beginSlice() const;
	int endSlice() const;

	void draw( QPainter &painter, int sliceIdx ) const;

private:
	int _begin;
	int _end;
};

#endif // MARROW_H
