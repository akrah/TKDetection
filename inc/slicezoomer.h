#ifndef SLICEZOOMER_H
#define SLICEZOOMER_H

#include <QObject>
#include <QPoint>

class SliceZoomer : public QObject
{
	Q_OBJECT

public:
	explicit SliceZoomer(QObject *parent = 0);

	qreal factor() const;

protected:
	bool eventFilter(QObject *, QEvent *event);

signals:
	void isMovedFrom( const QPoint &movementVector );
	void zoomFactorChanged( const qreal &newZoomFactor, const QPoint &focalPoint );

private:
	bool _isDraging;
	qreal _zoomFactor;
	QPoint _pointStartDrag;
};

#endif // SLICEZOOMER_H
