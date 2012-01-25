#include "inc/pointpolarseriesdata.h"

PointPolarSeriesData::PointPolarSeriesData() : QwtSeriesData<QwtPointPolar>() {
}

/*******************************
 * Public getters
 *******************************/

size_t PointPolarSeriesData::size() const {
	return _data.size();
}

QwtPointPolar PointPolarSeriesData::sample( size_t i ) const
{
	return _data[i%_data.size()];
}

QRectF PointPolarSeriesData::boundingRect() const
{
	if ( d_boundingRect.width() < 0.0 ) d_boundingRect = qwtBoundingRect( *this );
	return d_boundingRect;
}

/*******************************
 * Public setters
 *******************************/

void PointPolarSeriesData::append( QwtPointPolar point ) {
	_data.append(point);
}

void PointPolarSeriesData::clear() {
	_data.clear();
}
