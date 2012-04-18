#ifndef POINTPOLARSERIESDATA_H
#define POINTPOLARSERIESDATA_H

#include <qwt_series_data.h>

class PointPolarSeriesData: public QwtArraySeriesData<QwtPointPolar>
{
public:
	PointPolarSeriesData();

	virtual QRectF boundingRect() const;

	void append( QwtPointPolar point );
	void clear();
};

#endif // POINTPOLARSERIESDATA_H
