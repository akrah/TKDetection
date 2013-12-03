#include "inc/pointpolarseriesdata.h"

PointPolarSeriesData::PointPolarSeriesData( int size ) : QwtArraySeriesData<QwtPointPolar>()
{
	d_samples = QVector<QwtPointPolar>(size);
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

void PointPolarSeriesData::append( QwtPointPolar point )
{
	d_samples.append(point);
}

void PointPolarSeriesData::remove( const int &i )
{
	d_samples.remove(i);
}

void PointPolarSeriesData::remove( const int &i, const int &count )
{
	d_samples.remove(i,count);
}

void PointPolarSeriesData::resize( const int &size )
{
	d_samples.resize(size);
}

void PointPolarSeriesData::clear()
{
	d_boundingRect = QRectF( 0.0, 0.0, -1.0, -1.0 );
	d_samples.clear();
}

QVector<QwtPointPolar>::Iterator PointPolarSeriesData::begin()
{
	return d_samples.begin();
}

QwtPointPolar &PointPolarSeriesData::operator[]( int i )
{
	return d_samples[i];
}

const QwtPointPolar &PointPolarSeriesData::operator[]( int i ) const
{
	return d_samples[i];
}
