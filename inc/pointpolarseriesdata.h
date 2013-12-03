#ifndef POINTPOLARSERIESDATA_H
#define POINTPOLARSERIESDATA_H

#include <qwt_series_data.h>

class PointPolarSeriesData: public QwtArraySeriesData<QwtPointPolar>
{
public:
	PointPolarSeriesData( int size = 0 );

	virtual QRectF boundingRect() const;

	void append( QwtPointPolar point );

	void remove( const int &i );
	void remove( const int &i, const int &count );
	void resize( const int &size );
	void clear();

	QVector<QwtPointPolar>::Iterator begin();

	QwtPointPolar &operator[]( int i );
	const QwtPointPolar &operator[]( int i ) const;
};

#endif // POINTPOLARSERIESDATA_H
