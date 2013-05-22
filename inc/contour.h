#ifndef CONTOUR_H
#define CONTOUR_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "inc/coordinate.h"
#include "define.h"

#include <QVector>

class QPainter;

class Contour : public QVector<iCoord2D>
{
public:
	Contour();
	Contour( const QVector<iCoord2D> &vector );
	Contour( const Contour &contour );
	~Contour();

	void compute( const Slice &slice, const iCoord2D &sliceCenter, const int &intensityThreshold, iCoord2D startPoint = iCoord2D(-1,-1) );
	void smooth( int smoothingRadius );
	Contour convexHull();

	void draw( QPainter &painter, const int &cursorPosition, const iCoord2D &sliceCenter, const TKD::ViewType &viewType ) const;
};

#endif // CONTOUR_H
