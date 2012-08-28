#ifndef BILLON_H
#define BILLON_H

#include "global.h"
#include "marrow.h"

#include <QDebug>
#include <armadillo>

#include <QPolygon>

template< typename T >
class BillonTpl : public arma::Cube<T>
{
public:
	BillonTpl();
	BillonTpl( const int &width, const int &height, const int &depth );
	BillonTpl( const BillonTpl &billon );

	T minValue() const;
	T maxValue() const;
	qreal voxelWidth() const;
	qreal voxelHeight() const;
	qreal voxelDepth() const;

	void setMinValue( const T &value );
	void setMaxValue( const T &value );
	void setVoxelSize( const qreal &width, const qreal &height, const qreal &depth );

	QVector<rCoord2D> getAllRestrictedAreaVertex( const int &nbPolygonsPoints, const int &threshold, const Marrow *marrow = 0 ) const;
	qreal getRestrictedAreaBoudingBoxRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const;
	qreal getRestrictedAreaMeansRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const;

	iCoord2D findNearestPointOfThePith( const iCoord2D &center, const int &sliceNumber, const int &componentNumber ) const;
	QVector<iCoord2D> histogramOfNearestPointDistance( const Marrow &marrow, const int &componentNumber ) const;
	QVector<iCoord2D> extractContour( const iCoord2D &center, const int &sliceNumber, const int &componentNumber, iCoord2D startPoint = iCoord2D(-1,-1) ) const;

protected:
	T _minValue;
	T _maxValue;
	qreal _voxelWidth;	// Largeur d'un voxel en cm
	qreal _voxelHeight; // Hauteur d'un voxel en cm
	qreal _voxelDepth;	// Profondeur d'un voxel en cm
};

template< typename T >
BillonTpl<T>::BillonTpl() : arma::Cube<T>(), _minValue(static_cast<T>(0)), _maxValue(static_cast<T>(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const int &width, const int &height, const int &depth ) : arma::Cube<T>(height,width,depth), _minValue(static_cast<T>(0)), _maxValue(static_cast<T>(0)), _voxelWidth(0.), _voxelHeight(0.), _voxelDepth(0.) {}

template< typename T >
BillonTpl<T>::BillonTpl( const BillonTpl &billon ) : arma::Cube<T>(billon), _minValue(billon._minValue), _maxValue(billon._maxValue), _voxelWidth(billon._voxelWidth), _voxelHeight(billon._voxelHeight), _voxelDepth(billon._voxelDepth) {}

template< typename T >
T BillonTpl<T>::minValue() const {
	return _minValue;
}

template< typename T >
T BillonTpl<T>::maxValue() const {
	return _maxValue;
}

template< typename T >
qreal BillonTpl<T>::voxelWidth() const {
	return _voxelWidth;
}

template< typename T >
qreal BillonTpl<T>::voxelHeight() const {
	return _voxelHeight;
}

template< typename T >
qreal BillonTpl<T>::voxelDepth() const {
	return _voxelDepth;
}

template< typename T >
void BillonTpl<T>::setMinValue( const T &value ) {
	_minValue = value;
}

template< typename T >
void BillonTpl<T>::setMaxValue( const T &value ) {
	_maxValue = value;
}

template< typename T >
void BillonTpl<T>::setVoxelSize(const qreal &width, const qreal &height, const qreal &depth) {
	_voxelWidth = width;
	_voxelHeight = height;
	_voxelDepth = depth;
}

template< typename T >
QVector<rCoord2D> BillonTpl<T>::getAllRestrictedAreaVertex( const int &nbPolygonPoints, const int &threshold, const Marrow *marrow ) const {
	 QVector<rCoord2D> vectAllVertex;
	 const int nbSlices = this->n_slices;
	 const int thresholdRestrict = threshold-1;
	 for ( int indexSlice = 0 ; indexSlice<nbSlices ; ++indexSlice ) {
		 const arma::Mat<T> &currentSlice = this->slice(indexSlice);

		 const int sliceWidth = currentSlice.n_cols;
		 const int sliceHeight = currentSlice.n_rows;
		 const int xCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).x:sliceWidth/2;
		 const int yCenter = (marrow != 0 && marrow->interval().containsClosed(indexSlice))?marrow->at(indexSlice).y:sliceHeight/2;

		 qreal xEdge, yEdge, orientation, cosAngle, sinAngle;
		 orientation = 0.;
		 for ( int i=0 ; i<nbPolygonPoints ; ++i ) {
			 orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			 cosAngle = qCos(orientation);
			 sinAngle = -qSin(orientation);
			 xEdge = xCenter + 5*cosAngle;
			 yEdge = yCenter + 5*sinAngle;
			 while ( xEdge>0. && yEdge>0. && xEdge<sliceWidth && yEdge<sliceHeight && currentSlice.at(yEdge,xEdge) > thresholdRestrict ) {
					 xEdge += cosAngle;
					 yEdge += sinAngle;
			 }
			 vectAllVertex.push_back(rCoord2D(xEdge,yEdge));
		 }
	 }
	return vectAllVertex;
}

template< typename T >
qreal BillonTpl<T>::getRestrictedAreaBoudingBoxRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const
{
	QPolygon polygon(nbPolygonPoints);
	int polygonPoints[2*nbPolygonPoints+2];
	qreal xEdge, yEdge, orientation, cosAngle, sinAngle, radius;
	int i,k,counter;

	const int width = this->n_cols;
	const int height = this->n_rows;
	const int depth = this->n_slices;

	radius = 0.;
	for ( k=0 ; k<depth ; ++k ) {
		const arma::Mat<T> &currentSlice = this->slice(k);
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).x:width/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).y:height/2;

		orientation = 0.;
		counter = 0;
		for ( i=0 ; i<nbPolygonPoints ; ++i )
		{
			orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			cosAngle = qCos(orientation);
			sinAngle = -qSin(orientation);
			xEdge = xCenter + 10*cosAngle;
			yEdge = yCenter + 10*sinAngle;
			while ( xEdge>0 && yEdge>0 && xEdge<width && yEdge<height && currentSlice.at(yEdge,xEdge) > intensityThreshold )
			{
				xEdge += cosAngle;
				yEdge += sinAngle;
			}
			polygonPoints[counter++] = xEdge;
			polygonPoints[counter++] = yEdge;
		}
		polygonPoints[counter++] = polygonPoints[0];
		polygonPoints[counter] = polygonPoints[1];

		polygon.setPoints(nbPolygonPoints+1,polygonPoints);

		radius += 0.5*qSqrt(polygon.boundingRect().width()*polygon.boundingRect().width() + polygon.boundingRect().height()*polygon.boundingRect().height());
	}

	radius/=depth;
	qDebug() << "Rayon de la boite englobante : " << radius << " (" << radius*_voxelWidth << " mm)";
	return radius*_voxelWidth;
}

template< typename T >
qreal BillonTpl<T>::getRestrictedAreaMeansRadius( const Marrow *marrow, const int &nbPolygonPoints, int intensityThreshold ) const
{
	qreal xEdge, yEdge, orientation, cosAngle, sinAngle, radius;
	int i,k;

	const int width = this->n_cols;
	const int height = this->n_rows;
	const int depth = this->n_slices;

	radius = 0.;
	for ( k=0 ; k<depth ; ++k ) {
		const arma::Mat<T> &currentSlice = this->slice(k);
		const int xCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).x:width/2;
		const int yCenter = (marrow != 0 && marrow->interval().containsClosed(k))?marrow->at(k-marrow->interval().minValue()).y:height/2;

		orientation = 0.;
		for ( i=0 ; i<nbPolygonPoints ; ++i )
		{
			orientation += (TWO_PI/static_cast<qreal>(nbPolygonPoints));
			cosAngle = qCos(orientation);
			sinAngle = -qSin(orientation);
			xEdge = xCenter + 10*cosAngle;
			yEdge = yCenter + 10*sinAngle;
			while ( xEdge>0 && yEdge>0 && xEdge<width && yEdge<height && currentSlice.at(yEdge,xEdge) > intensityThreshold )
			{
				xEdge += cosAngle;
				yEdge += sinAngle;
			}
			xEdge -= xCenter;
			yEdge -= yCenter;
			radius += qSqrt( xEdge*xEdge + yEdge*yEdge );
		}
	}

	radius/=(depth*nbPolygonPoints);
	qDebug() << "Rayon de la boite englobante (en pixels) : " << radius;
	return radius;
}

//template < typename T >
//iCoord2D BillonTpl<T>::findNearestPointOfThePith( const Marrow *marrow, const int &sliceNumber, const int &componentNumber ) const
//{
//	// Find the pixel closest to the pith
//	const arma::Mat<T> &currentSlice = this->slice(sliceNumber);
//	const int width = this->n_cols;
//	const int height = this->n_rows;
//	const int xCenter = marrow != 0 ? marrow->at(sliceNumber).x : width/2;
//	const int yCenter = marrow != 0 ? marrow->at(sliceNumber).y : height/2;

//	rCoord2D position;
//	qreal radiusMax, orientation, step;
//	bool edgeFind = false;
//	radiusMax = qMin( qMin(xCenter,width-xCenter), qMin(yCenter,height-yCenter) );

//	step = 0.;
//	while ( !edgeFind && step < radiusMax-1 )
//	{
//		step+=0.5;
//		for ( orientation = 0. ; orientation <= TWO_PI && !edgeFind ; orientation+=(1./(2*PI*radiusMax)) )
//		{
//			position.x = xCenter + step*qCos(orientation);
//			position.y = yCenter - step*qSin(orientation);
//			if ( currentSlice.at(position.y,position.x) == componentNumber ) edgeFind = true;
//		}
//	}

//	if ( edgeFind ) {
//		qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
//	}
//	else {
//		qDebug() << "Aucun pixel et donc aucune composante connexe";
//		position.x = 0;
//		position.y = 0;
//	}

//	return iCoord2D(position.x,position.y);
//}

template < typename T >
iCoord2D BillonTpl<T>::findNearestPointOfThePith( const iCoord2D &center, const int &sliceNumber, const int &componentNumber ) const
{
	// Find the pixel closest to the pith
	const arma::Mat<T> &currentSlice = this->slice(sliceNumber);
	const int width = this->n_cols;
	const int height = this->n_rows;
	const int xCenter = center.x;
	const int yCenter = center.y;
	const int radiusMax = qMin( qMin(xCenter,width-xCenter), qMin(yCenter,height-yCenter) );

	rCoord2D position;
	bool edgeFind = false;
	int currentRadius, x, y, d;

	currentRadius = 1;
	while ( !edgeFind && currentRadius < radiusMax )
	{
		x = 0;
		y = currentRadius;
		d = currentRadius - 1;
		while ( y>=x && !edgeFind )
		{
			edgeFind = true;
			if ( currentSlice.at( yCenter+y, xCenter+x ) == componentNumber )
			{
				position.x = xCenter+x;
				position.y = yCenter+y;
			}
			else if ( currentSlice.at( yCenter+y, xCenter-x ) == componentNumber )
			{
				position.x = xCenter-x;
				position.y = yCenter+y;
			}
			else if ( currentSlice.at( yCenter+x, xCenter+y ) == componentNumber )
			{
				position.x = xCenter+y;
				position.y = yCenter+x;
			}
			else if ( currentSlice.at( yCenter+x, xCenter-y ) == componentNumber )
			{
				position.x = xCenter-y;
				position.y = yCenter+x;
			}
			else if ( currentSlice.at( yCenter-y, xCenter+x ) == componentNumber )
			{
				position.x = xCenter+x;
				position.y = yCenter-y;
			}
			else if ( currentSlice.at( yCenter-y, xCenter-x ) == componentNumber )
			{
				position.x = xCenter-x;
				position.y = yCenter-y;
			}
			else if ( currentSlice.at( yCenter-x, xCenter+y ) == componentNumber )
			{
				position.x = xCenter+y;
				position.y = yCenter-x;
			}
			else if ( currentSlice.at( yCenter-x, xCenter-y ) == componentNumber )
			{
				position.x = xCenter-y;
				position.y = yCenter-x;
			}
			else
			{
				edgeFind = false;
				if ( d >= 2*x )
				{
					d -= 2*x;
					x++;
				}
				else if ( d <= 2*(currentRadius-y) )
				{
					d += 2*y;
					y--;
				}
				else
				{
					d += 2*(y-x);
					y--;
					x++;
				}
			}
		}
		currentRadius++;
	}

	if ( edgeFind ) {
		qDebug() << "Pixel le plus proche de la moelle : ( " << position.x << ", " << position.y << " )";
	}
	else {
		qDebug() << "Aucun pixel et donc aucune composante connexe";
		position.x = -1;
		position.y = -1;
	}

	return iCoord2D(position.x,position.y);
}

template< typename T >
QVector<iCoord2D> BillonTpl<T>::histogramOfNearestPointDistance( const Marrow &marrow, const int &componentNumber ) const
{
	const int depth = this->n_slices;
	QVector<iCoord2D> distanceBySlice(this->n_slices,iCoord2D(0,0));
	for ( int i=0 ; i<depth ; ++i )
	{
		distanceBySlice[i] = iCoord2D(i,findNearestPointOfThePith(marrow[i],i,componentNumber).distance(marrow[i]));
	}
	return distanceBySlice;
}

template< typename T >
QVector<iCoord2D> BillonTpl<T>::extractContour( const iCoord2D &center, const int &sliceNumber, const int &componentNumber, iCoord2D startPoint ) const
{
	if ( startPoint == iCoord2D(-1,-1) )
	{
		startPoint = findNearestPointOfThePith( center, sliceNumber, componentNumber );
	}

	// Suivi du contour
	QVector<iCoord2D> contourPoints;
	if ( startPoint != iCoord2D(-1,-1) )
	{
		const arma::Mat<T> &currentSlice = this->slice(sliceNumber);
		const int xCenter = center.x;
		const int yCenter = center.y;
		qreal orientation = ANGLE(xCenter,yCenter,startPoint.x,startPoint.y);

		int xBegin, yBegin, xCurrent, yCurrent, interdit, j;
		QVector<int> vx(8), vy(8);

		xBegin = xCurrent = startPoint.x;
		yBegin = yCurrent = startPoint.y;
		interdit = orientation*8./TWO_PI;
		interdit = (interdit+4)%8;
		do
		{
			contourPoints.append(iCoord2D(xCurrent,yCurrent));
			vx[0] = vx[1] = vx[7] = xCurrent+1;
			vx[2] = vx[6] = xCurrent;
			vx[3] = vx[4] = vx[5] = xCurrent-1;
			vy[1] = vy[2] = vy[3] = yCurrent+1;
			vy[0] = vy[4] = yCurrent;
			vy[5] = vy[6] = vy[7] = yCurrent-1;
			j = (interdit+1)%8;
			while ( currentSlice.at(vy[j%8],vx[j%8]) != componentNumber && j < interdit+8 ) ++j;
			xCurrent = vx[j%8];
			yCurrent = vy[j%8];
			interdit = (j+4)%8;
		}
		while ( xBegin != xCurrent || yBegin != yCurrent );
	}

	return contourPoints;
}


#endif // BILLON_H
