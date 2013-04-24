#include "inc/slicezoomer.h"

#include <QMouseEvent>
#include <QDebug>

SliceZoomer::SliceZoomer(QObject *parent) : QObject(parent), _isDraging(false), _zoomFactor(1), _pointStartDrag(-1,-1) {
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
				if ( _zoomFactor > 0.1 && wheelDelta )
				{
					_zoomFactor *= wheelDelta>0 ? ZOOM_COEF_IN : ZOOM_COEF_OUT;
					_zoomCoefficient = wheelDelta>0 ? ZOOM_COEF_IN : ZOOM_COEF_OUT;
					emit zoomFactorChanged(_zoomFactor,_zoomCoefficient);
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
	_zoomCoefficient = _zoomFactor<1. ? ZOOM_COEF_IN*(1./(ZOOM_COEF_IN*_zoomFactor)) : ZOOM_COEF_OUT*(1./(ZOOM_COEF_IN*_zoomFactor));
	_zoomFactor = 1.;
	emit zoomFactorChanged(_zoomFactor,_zoomCoefficient);
}
