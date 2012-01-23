#ifndef POINTPOLARSERIESDATA_H
#define POINTPOLARSERIESDATA_H

#include <qwt_series_data.h>

class PointPolarSeriesData: public QwtSeriesData<QwtPointPolar>
{
public:
	PointPolarSeriesData();

	virtual size_t size() const;
	virtual QwtPointPolar sample( size_t i ) const;
	virtual QRectF boundingRect() const;

	void append( QwtPointPolar point );
	void clear();

protected:
	QVector<QwtPointPolar> _data;
};

#endif // POINTPOLARSERIESDATA_H
