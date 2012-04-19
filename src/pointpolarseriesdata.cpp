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

void PointPolarSeriesData::setSamples( const QVector<QwtPointPolar> &samples ) {
	d_boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
	d_samples = samples;
}

void PointPolarSeriesData::append( QwtPointPolar point ) {
	d_samples.append(point);
}

void PointPolarSeriesData::remove( const int &i ) {
	d_samples.remove(i);
}

void PointPolarSeriesData::remove( const int &i, const int &count ) {
	d_samples.remove(i,count);
}

void PointPolarSeriesData::clear() {
	d_boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
	d_samples.clear();
}
