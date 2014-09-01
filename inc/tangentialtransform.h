#ifndef TANGENTIALTRANSFORM_H
#define TANGENTIALTRANSFORM_H

#include "def/def_billon.h"
#include "def/def_coordinate.h"
#include "interval.h"
#include "inc/coordinate.h"

#include <QtGlobal>
#include <QQuaternion>

class TangentialTransform
{
public:
	TangentialTransform(const int &minIntensity, const bool &trilinearInterpolation );
	TangentialTransform( const TangentialTransform &tangentialTransform );
	virtual ~TangentialTransform();

	Billon* execute(const Billon &billon );

	void setSliceInterval( const Billon &billon, const Interval<uint> &sliceInterval );
	void setAngularInterval( const Billon &billon, const Interval<uint> &angularInterval );
	void updateIntervals( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &angularInterval );

	void setMinIntensity( const int &minIntensity );
	void enableTrilinearInterpolation( bool enable );

	const int &minIntensity() const;
	const bool &trilinearInterpolation() const;

	const Interval<uint> &currentSliceInterval() const;
	const Interval<uint> &currentAngularInterval() const;

	const QVector3D &origin() const;
	QVector3D originRelativeToSliceInterval() const;
	const qreal &angularRange() const;
	const qreal &bisectorOrientation() const;
	const qreal &depth() const;

	const QQuaternion &quaterX() const;
	const QQuaternion &quaterY() const;
	const QQuaternion &quaterZ() const;
	const QQuaternion &quaterRot() const;

	const QVector3D &shiftStep() const;
	QVector3D shiftStep( const qreal &stepInZ ) const;

protected:
	int _minIntensity;
	bool _trilinearInterpolation;

	Interval<uint> _currentSliceInterval;
	Interval<uint> _currentAngularInterval;

	QVector3D _origin;
	qreal _angularRange;
	qreal _bisectorOrientation;
	qreal _depth;

	QQuaternion _quaterX;
	QQuaternion _quaterY;
	QQuaternion _quaterZ;
	QQuaternion _quaterRot;

	QVector3D _shiftStep;
};

#endif // TANGENTIALTRANSFORM_H
