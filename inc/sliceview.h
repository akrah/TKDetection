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

	SliceType::SliceType sliceType() const;

	void setModel( const Billon * billon );

public slots:
	void drawSlice( QPainter &painter, const int &sliceNumber );
	void setLowThreshold( const int &threshold );
	void setHighThreshold( const int &threshold );
	void setTypeOfView( const int &type = SliceType::CURRENT );

private :
	void drawCurrentSlice( QPainter &painter, const int &sliceNumber );
	void drawAverageSlice( QPainter &painter );
	void drawMedianSlice( QPainter &painter );

signals:
	void thresholdUpdated();
	void typeOfViewChanged( const SliceType::SliceType & );

private:
	const Billon * _billon;

	int _lowThreshold;
	int _highThreshold;
	SliceType::SliceType _typeOfView;
};

#endif // SLICEVIEW_H
