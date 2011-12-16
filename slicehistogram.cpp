#include "slicehistogram.h"

#include "billon.h"
#include <QPainter>

SliceHistogram::SliceHistogram( QWidget *parent ) : QWidget(parent), _billon(0) {
}

void SliceHistogram::setModel( const Billon *billon ) {
	_billon = billon;
	_graph.clear();
	update();
}

void SliceHistogram::update() {
	if ( _billon != 0 ) {
		const uint width = _billon->n_cols;
		const uint height = _billon->n_rows;
		const uint depth = _billon->n_slices;
		const float min = _billon->minValue();

		_graph.clear();
		_graph.reserve(depth);
		setFixedSize(width,HISTOGRAM_HEIGHT);

		float cumul, max;
		max = 0;
		for (unsigned int k=0 ; k<depth ; k++) {
			const imat &slice = _billon->slice(k);
			cumul = 0;
			for (unsigned int j=0 ; j<height ; j++) {
				for (unsigned int i=0 ; i<width ; i++) {
					cumul += (slice.at(j,i)-min);
				}
			}
			_graph.push_back(cumul);
			if ( cumul>max ) max = cumul;
		}

		const float fact = 200.0/max;
		for ( unsigned int i=0 ; i<depth ; i++ ) {
			_graph[i] = _graph[i]*fact;
		}
	}
	repaint();
}

void SliceHistogram::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const int graphSize = _graph.size();
	if ( graphSize == 0 ) {
		painter.drawLine(0,0,((QWidget*)parent())->width(),((QWidget*)parent())->height());
		painter.drawLine(((QWidget*)parent())->width(),0,0,((QWidget*)parent())->height());
	}
	else {
		const float stickWidth = (float)width()/(float)graphSize;

//		// Dessiner la courbe
		for ( int i=0 ; i<graphSize-1 ; i++ ) {
			painter.drawLine(i*stickWidth,HISTOGRAM_HEIGHT-_graph[i],(i+1)*stickWidth,HISTOGRAM_HEIGHT-_graph[i+1]);
		}

		// Dessiner un graphe plein
//		for ( int i=0 ; i<graphSize ; i++ ) {
//			//painter.drawLine((i+0.5)*stickWidth,HISTOGRAM_HEIGHT-_graph[i],(i+0.5)*stickWidth,HISTOGRAM_HEIGHT);
//			painter.drawRect(i*stickWidth,HISTOGRAM_HEIGHT-_graph[i],stickWidth,_graph[i]);
//		}
	}
}
