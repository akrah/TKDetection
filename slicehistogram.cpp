#include "slicehistogram.h"

#include "billon.h"
#include <QPainter>

SliceHistogram::SliceHistogram(QWidget *parent) : QWidget(parent) {
}

void SliceHistogram::update(const Billon &billon) {
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const float min = billon.minValue();

	_graph.clear();
	setFixedSize(width,HISTOGRAM_HEIGHT);

	int cumul;
	int max = 0;
	for (unsigned int k=0 ; k<depth ; k++) {
		cumul = 0.;
		for (unsigned int j=0 ; j<height ; j++) {
			for (unsigned int i=0 ; i<width ; i++) {
				cumul += (billon.slice(k)(j,i)-min);
			}
		}
		_graph.append(cumul);
		max = (cumul>max)?cumul:max;
	}

	const float fact = 200./(float)max;
	for ( int i=0 ; i<_graph.size() ; i++ ) {
		_graph[i] = _graph[i] * fact;
	}
	repaint();
}

void SliceHistogram::paintEvent(QPaintEvent *event) {
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	const int histogramWidth = width();
	std::cout << histogramWidth << std::endl;

	if ( _graph.isEmpty() ) {
		painter.drawEllipse(0,0,histogramWidth,HISTOGRAM_HEIGHT);
	}
	else {
		const int graphSize = _graph.size();
		const float stickWidth = (float)histogramWidth/(float)graphSize;

		// Dessiner la courbe
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
