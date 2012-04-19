#ifndef POINTPOLARSERIESDATA_H
#define POINTPOLARSERIESDATA_H

#include <qwt_series_data.h>

class PointPolarSeriesData: public QwtArraySeriesData<QwtPointPolar>
{
public:
	PointPolarSeriesData();

	virtual QRectF boundingRect() const;

	void setSamples( const QVector<QwtPointPolar> &samples );
	void append( QwtPointPolar point );

	void remove( const int &i );
	void remove( const int &i, const int &count );
	void clear();
};

#endif // POINTPOLARSERIESDATA_H
