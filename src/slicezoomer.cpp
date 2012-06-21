#include "inc/slicezoomer.h"

#include <QMouseEvent>
#include <QDebug>

SliceZoomer::SliceZoomer(QObject *parent) : QObject(parent), _isDraging(false), _zoomFactor(1), _pointStartDrag(-1,-1) {
}

qreal SliceZoomer::factor() const {
	return _zoomFactor;
}

bool SliceZoomer::eventFilter(QObject *, QEvent *event) {
	bool isFiltered = false;
	switch ( event->type() ) {
		case QEvent::MouseButtonPress :
			{
				_isDraging = true;
				_pointStartDrag = static_cast<const QMouseEvent*>(event)->pos();
			}
			break;
		case QEvent::MouseMove :
			{
				if ( _isDraging ) {
					const QMouseEvent *mouseEvent = static_cast<const QMouseEvent*>(event);
					const QPoint movementVector = mouseEvent->pos() - _pointStartDrag;
					if ( movementVector.manhattanLength() > 0 ) {
						_pointStartDrag = mouseEvent->pos();
						emit isMovedFrom(movementVector);
					}
					isFiltered = true;
				}
			}
			break;
		case QEvent::MouseButtonRelease :
			{
				_isDraging = false;
				isFiltered = true;
			}
			break;
		case QEvent::Wheel :
			{
				const QWheelEvent *wheelEvent = static_cast<const QWheelEvent*>(event);
				const int wheelDelta = wheelEvent->delta();
				if ( wheelDelta != 0 ) {
					_zoomFactor += wheelDelta>0 ? 0.10 : -0.10;
					emit zoomFactorChanged(_zoomFactor,wheelEvent->globalPos());
				}
				isFiltered = true;
			}
			break;
		default :
			break;
	}
	return isFiltered;
}
