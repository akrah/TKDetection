#ifndef CONTOUR_H
#define CONTOUR_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/coordinate.h"

#include <QVector>

class QImage;

class Contour : public QVector<iCoord2D>
{
public:
	Contour();
	Contour( const QVector<iCoord2D> &vector );
	Contour( const Contour &contour );
	~Contour();

	void compute( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, iCoord2D startPoint = iCoord2D(-1,-1) );
	void smooth( int smoothingRadius );

	void draw( QImage &image, const int &cursorPosition ) const;
};

#endif // CONTOUR_H
