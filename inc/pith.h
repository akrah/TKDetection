#ifndef MARROW_H
#define MARROW_H

#include "def/def_coordinate.h"

#include <QVector>

class QImage;

class Pith : public QVector<rCoord2D>
{
public:
	Pith( const int size = 0 );
	Pith( const Pith &pith );
	Pith( const QVector<rCoord2D> &coordinates );

	void draw( QImage &image, const int &sliceIdx , const int &radius = 5 ) const;
};

#endif // MARROW_H
