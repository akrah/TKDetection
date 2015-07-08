#ifndef SLICEZOOMER_H
#define SLICEZOOMER_H

#include <QObject>
#include <QPoint>

#define ZOOM_COEF_IN    1.25
#define ZOOM_COEF_OUT    .8
#define ZOOM_OUT_ON_IN   .64

#define ZOOM_MAX_FACTOR 8.
#define ZOOM_MIN_FACTOR  .1

class SliceZoomer : public QObject
{
	Q_OBJECT

public:
	explicit SliceZoomer(QObject *parent = 0);

	qreal factor() const;
	qreal coefficient() const;

protected:
	bool eventFilter(QObject *, QEvent *event);

public slots:
	void resetZoom();

signals:
	void isMovedFrom( const QPoint &movementVector );
	void zoomFactorChanged( const qreal &newZoomFactor, const qreal &newZoomCoefficient );

private:
	bool _isDraging;
	qreal _zoomFactor;
	qreal _zoomCoefficient;
	QPoint _pointStartDrag;
};

#endif // SLICEZOOMER_H
