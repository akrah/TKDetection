#ifndef OLDKNOTAREADETECTOR_H
#define OLDKNOTAREADETECTOR_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"

#include <QtGlobal>
#include <QVector>
#include <QRect>
#include <QMap>

class OldKnotAreaDetector
{
public:
	OldKnotAreaDetector();

	void execute( const Slice &accumulationSlice );

	void computeLabelledImage( const Slice &accumulationSlice, Slice &labelledSlice , QMap<int, QList<iCoord2D> > &ccList );
	void findNearestConnectedComponent( const QList<iCoord2D> &currentCC, const QMap<int, QList<iCoord2D> > &supportingAreaList , int &ccMin, qreal &minDist );

	const __billon_type__ &binarizationThreshold() const;
	const qreal &maximumConnectedComponentDistance() const;
	const uint &minimumConnectedComponentSize() const;
	const QVector<QRect> &supportingAreaVector() const;

	bool hasSupportingAreas() const;

	void setBinarizationThreshold( const __billon_type__ &newThreshold );
	void setMaximumConnectedComponentDistance( const qreal &newDistance );
	void setMinimumConnectedComponentSize( const uint &size );
	void clearKnotAreas();


private:
	__billon_type__ _binarizationThreshold;
	qreal _maximumConnectedComponentDistance;
	uint _minimumConnectedComponentSize;

	QVector<QRect> _supportingAreaVector;
};

#endif // OLDKNOTAREADETECTOR_H
