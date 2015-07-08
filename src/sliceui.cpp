#include "inc/sliceui.h"

#include <QScrollBar>

SliceUI::SliceUI( QWidget * parent ) : QScrollArea(parent)
{
	_label.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	_label.setScaledContents(true);
	_label.installEventFilter(&_zoomer);
//	_lab.installEventFilter(this);

	setBackgroundRole(QPalette::Dark);
	setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	setFrameShape(QFrame::NoFrame);
	setFrameShadow(QFrame::Plain);
	setLineWidth(0);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setWidgetResizable(false);
	setAlignment(Qt::AlignCenter);
	setWidget(&_label);

	QObject::connect(&_zoomer, SIGNAL(zoomFactorChanged(qreal,qreal)), this, SLOT(zoom(qreal,qreal)));
	QObject::connect(&_zoomer, SIGNAL(isMovedFrom(QPoint)), this, SLOT(drag(QPoint)));
}

SliceUI::~SliceUI() {}

void SliceUI::installLabelEventFilter( QObject *object )
{
	_label.installEventFilter(object);
}

const QLabel &SliceUI::label() const
{
	return _label;
}

const QImage &SliceUI::image() const
{
	return _image;
}

const SliceZoomer &SliceUI::zoomer() const
{
	return _zoomer;
}

QImage &SliceUI::image()
{
	return _image;
}

void SliceUI::updateLabel()
{
	_label.resize( _zoomer.factor() * _image.size() );
	_label.setPixmap( QPixmap::fromImage(_image).scaledToWidth(_zoomer.factor()*_image.width(),Qt::FastTransformation) );
}

void SliceUI::resizeBlackImage( const uint &width, const uint &height )
{
	_image = QImage(width,height,QImage::Format_ARGB32);
	_image.fill(0xff000000);
}

void SliceUI::zoom( const qreal &zoomFactor, const qreal &zoomCoefficient )
{
	_label.resize(zoomFactor * _image.size());
	_label.setPixmap( QPixmap::fromImage(_image).scaledToWidth(zoomFactor*_image.width(),Qt::FastTransformation) );
	QScrollBar *hBar = this->horizontalScrollBar();
	hBar->setValue(int(zoomCoefficient * hBar->value() + ((zoomCoefficient - 1) * hBar->pageStep()/2)));
	QScrollBar *vBar = this->verticalScrollBar();
	vBar->setValue(int(zoomCoefficient * vBar->value() + ((zoomCoefficient - 1) * vBar->pageStep()/2)));
}

void SliceUI::drag( const QPoint &movementVector )
{
	QScrollArea &scrollArea = *this;
	if ( movementVector.x() ) scrollArea.horizontalScrollBar()->setValue(scrollArea.horizontalScrollBar()->value()-movementVector.x());
	if ( movementVector.y() ) scrollArea.verticalScrollBar()->setValue(scrollArea.verticalScrollBar()->value()-movementVector.y());
}
