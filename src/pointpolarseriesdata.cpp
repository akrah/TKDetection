#include "inc/pointpolarseriesdata.h"

PointPolarSeriesData::PointPolarSeriesData() : QwtArraySeriesData<QwtPointPolar>() {
}

/*******************************
 * Public getters
 *******************************/

QRectF PointPolarSeriesData::boundingRect() const
{
	if ( d_boundingRect.width() < 0.0 ) d_boundingRect = qwtBoundingRect( *this );
	return d_boundingRect;
}

/*******************************
 * Public setters
 *******************************/

void PointPolarSeriesData::append( QwtPointPolar point ) {
	d_samples.append(point);
}

void PointPolarSeriesData::clear() {
	d_samples.clear();
}
