#ifndef SLICEUI_H
#define SLICEUI_H

#include <QScrollArea>
#include <QLabel>
#include <QImage>
#include "inc/slicezoomer.h"

class SliceUI : public QScrollArea
{

Q_OBJECT

public:
	explicit SliceUI( QWidget * parent = 0 );
	virtual ~SliceUI();

	const QLabel &label() const;
	const QImage &image() const;
	const SliceZoomer &zoomer() const;

	QImage &image();

	void installLabelEventFilter( QObject *object );

	void updateLabel();
	void resizeBlackImage( const uint &width, const uint &height );

private slots:
	void zoom( const qreal &zoomFactor, const qreal &zoomCoefficient );
	void drag( const QPoint &movementVector );

private:
	QLabel _label;
	QImage _image;
	SliceZoomer _zoomer;
};

#endif // SLICEUI_H
