#ifndef SLICEVIEW_H
#define SLICEVIEW_H

#include "sliceview_def.h"
#include <QObject>

class Billon;
class QPixmap;
class Marrow;
class QPainter;

class SliceView : public QObject
{
	Q_OBJECT

public:
	explicit SliceView();
	~SliceView();

	void setModel( const Billon * billon );
	void setTypeOfView( const SliceType::SliceType &type );

public slots:
	void drawSlice( QPainter &painter, const int &sliceNumber );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );

private :
	void drawCurrentSlice( QPainter &painter, const int &sliceNumber );
	void drawAverageSlice( QPainter &painter );
	void drawMedianSlice( QPainter &painter );

private:
	const Billon * _billon;

	int _lowThreshold;
	int _highThreshold;
	SliceType::SliceType _typeOfView;
};

#endif // SLICEVIEW_H
