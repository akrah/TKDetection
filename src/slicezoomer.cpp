#include "inc/slicezoomer.h"

#include <QMouseEvent>
#include <QDebug>

SliceZoomer::SliceZoomer(QObject *parent) : QObject(parent), _isDraging(false), _zoomFactor(1), _zoomCoefficient(0), _pointStartDrag(-1,-1) {
}

qreal SliceZoomer::factor() const
{
	return _zoomFactor;
}

qreal SliceZoomer::coefficient() const
{
	return _zoomCoefficient;
}

bool SliceZoomer::eventFilter(QObject *obj, QEvent *event)
{
	switch ( event->type() )
	{
		case QEvent::MouseButtonPress :
			{
				_isDraging = true;
				_pointStartDrag = static_cast<const QMouseEvent*>(event)->pos();
			}
			break;
		case QEvent::MouseMove :
			{
				if ( _isDraging )
				{
					const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
					const QPoint movementVector = mouseEvent->pos() - _pointStartDrag;
					if ( movementVector.manhattanLength() > 0 )
					{
						_pointStartDrag = mouseEvent->pos();
						emit isMovedFrom(movementVector);
					}
				}
			}
			break;
		case QEvent::MouseButtonRelease :
			{
				_isDraging = false;
			}
			break;
		case QEvent::Wheel :
			{
				const QWheelEvent *wheelEvent = static_cast<const QWheelEvent*>(event);
				const int wheelDelta = wheelEvent->delta();
				if ( wheelDelta )
				{
					const qreal newZoomCoefficient = wheelDelta>0 ? ZOOM_COEF_IN : ZOOM_COEF_OUT;
					const qreal newZoomFactor = _zoomFactor * newZoomCoefficient;
					if ( newZoomFactor > ZOOM_MIN_FACTOR && newZoomFactor < ZOOM_MAX_FACTOR )
					{
						_zoomCoefficient = newZoomCoefficient;
						_zoomFactor = newZoomFactor;
						emit zoomFactorChanged(newZoomFactor,newZoomCoefficient);
					}
				}
			}
			break;
		default :
			break;
	}
	return QObject::eventFilter(obj,event);
}

void SliceZoomer::resetZoom()
{
	_isDraging = false;
	_zoomCoefficient = 1./_zoomFactor;
	_zoomFactor = 1.;
	emit zoomFactorChanged(_zoomFactor,_zoomCoefficient);
}
