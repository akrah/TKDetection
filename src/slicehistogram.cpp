#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/intensityinterval.h"
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

SliceHistogram::SliceHistogram() : _histogram(new QwtPlotHistogram()), _histogramMaximums(new QwtPlotHistogram()), _histogramMinimums(new QwtPlotHistogram()), _histogramBranchesArea(new QwtPlotHistogram()),
	_curveMeans(new QwtPlotCurve()), _dataMeans(0.), _curveMedian(new QwtPlotCurve()), _dataMedian(0.), _marrowAroundDiameter(20), _intervalType(HistogramIntervalType::FROM_EDGE),
	_minimumIntervalWidth(0), _movementThresholdMin(0), _movementThresholdMax(1000), _smoothing(true), _useNextSlice(false)
{
	_histogramMaximums->setBrush(Qt::green);
	_histogramMaximums->setPen(QPen(Qt::green));

	_histogramBranchesArea->setBrush(Qt::blue);
	_histogramBranchesArea->setPen(QPen(Qt::blue));

	_histogramMinimums->setBrush(Qt::yellow);
	_histogramMinimums->setPen(QPen(Qt::yellow));

	_curveMeans->setPen(QPen(Qt::red));
	_curveMedian->setPen(QPen(Qt::cyan));
}

SliceHistogram::~SliceHistogram() {
	if ( _histogram != 0 ) delete _histogram;
	if ( _histogramMaximums != 0 ) delete _histogramMaximums;
	if ( _histogramMinimums != 0 ) delete _histogramMinimums;
	if ( _histogramBranchesArea != 0 ) delete _histogramBranchesArea;
	if ( _curveMeans != 0 ) delete _curveMeans;
	if ( _curveMedian != 0 ) delete _curveMedian;
}

/*******************************
 * Public getters
 *******************************/

qreal SliceHistogram::value( const int &index ) const {
	qreal res = 0.;
	if ( index > -1 && _datasHistogram.size() > index ) {
		res = _datasHistogram.at(index).value;
	}
	return res;
}

int SliceHistogram::nbMaximums() const {
	int nbMaximums = _histogramMaximums == 0 ? 0 : _histogramMaximums->dataSize();
	return nbMaximums;
}

int SliceHistogram::sliceOfIemeMaximum( const int &maximumIndex ) const {
	int sliceIndex = -1;
	if ( _histogramMaximums != 0 && maximumIndex>-1 && maximumIndex<static_cast<int>(_histogramMaximums->dataSize()) ) {
		sliceIndex = static_cast<QwtIntervalSample>(_histogramMaximums->data()->sample(maximumIndex)).interval.minValue();
	}
	return sliceIndex;
}

int SliceHistogram::marrowAroundDiameter() const {
	return _marrowAroundDiameter;
}

const QVector<QwtIntervalSample> & SliceHistogram::branchesAreas() const {
	return _datasBranchesRealAreas;
}

/*******************************
 * Public setters
 *******************************/
void SliceHistogram::setMarrowAroundDiameter( const int &diameter ) {
	_marrowAroundDiameter = diameter;
}

void SliceHistogram::setIntervalType( const HistogramIntervalType::HistogramIntervalType &type ) {
	if ( type > HistogramIntervalType::_HIST_INTERVAL_TYPE_MIN_ && type < HistogramIntervalType::_HIST_INTERVAL_TYPE_MAX__ ) {
		_intervalType = type;
	}
}

void SliceHistogram::setMinimumIntervalWidth( const int &width ) {
	_minimumIntervalWidth = width;
}

void SliceHistogram::setMovementThresholdMin( const int &threshold ) {
	_movementThresholdMin = threshold;
}

void SliceHistogram::setMovementThresholdMax( const int &threshold ) {
	_movementThresholdMax = threshold;
}

void SliceHistogram::enableSmoothing( const bool &enable ) {
	_smoothing = enable;
}

void SliceHistogram::useNextSlice( const bool &enable ) {
	_useNextSlice = enable;
}


void SliceHistogram::attach( QwtPlot * const plot ) {
	if ( plot != 0 ) {
		if ( _histogram != 0 ) {
			_histogram->attach(plot);
		}
		if ( _histogramBranchesArea != 0 ) {
			_histogramBranchesArea->attach(plot);
		}
		if ( _histogramMaximums != 0 ) {
			_histogramMaximums->attach(plot);
		}
		if ( _histogramMinimums != 0 ) {
			_histogramMinimums->attach(plot);
		}
		if ( _curveMeans != 0 ) {
			_curveMeans->attach(plot);
		}
		if ( _curveMedian != 0 ) {
			_curveMedian->attach(plot);
		}
	}
}

void SliceHistogram::detach() {
	if ( _histogram != 0 ) {
		_histogram->detach();
	}
	if ( _histogramBranchesArea != 0 ) {
		_histogramBranchesArea->detach();
	}
	if ( _histogramMaximums != 0 ) {
		_histogramMaximums->detach();
	}
	if ( _histogramMinimums != 0 ) {
		_histogramMinimums->detach();
	}
	if ( _curveMeans != 0 ) {
		_curveMeans->detach();
	}
	if ( _curveMedian != 0 ) {
		_curveMedian->detach();
	}
}

void SliceHistogram::clear() {
	_datasHistogram.clear();
	_histogram->setSamples(_datasHistogram);
	_datasMaximums.clear();
	_histogramMaximums->setSamples(_datasMaximums);
	_datasMinimums.clear();
	_histogramMinimums->setSamples(_datasMinimums);
	_datasBranchesAreaToDrawing.clear();
	_histogramBranchesArea->setSamples(_datasBranchesAreaToDrawing);
	_dataMeans = 0.;
	_curveMeans->setSamples(QVector<QPointF>());
	_dataMedian = 0.;
	_curveMedian->setSamples(QVector<QPointF>());
}

void SliceHistogram::constructHistogram( const Billon &billon ) {
	_datasHistogram.clear();

	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = _useNextSlice?billon.n_slices-1:billon.n_slices;
	const int minValue = billon.minValue();

	_datasHistogram.reserve(depth-1);

	qreal cumul;
	qreal diff;
	int currentSliceValue, previousSliceValue, nbPixels;
	for ( uint k=1 ; k<depth ; ++k ) {
		const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		cumul = 0;
		nbPixels = 0;
		for ( uint j=0 ; j<height ; ++j ) {
			for ( uint i=0 ; i<width ; ++i ) {
				currentSliceValue = slice.at(j,i);
				previousSliceValue = prevSlice.at(j,i);
				if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
					diff = qAbs(currentSliceValue - previousSliceValue);
					if ( (diff > _movementThresholdMin) && (diff < _movementThresholdMax) ) {
						cumul += diff;
					}
					nbPixels++;
				}
			}
		}
		if ( nbPixels > 0 ) _datasHistogram.append(QwtIntervalSample(cumul,k-1,k));
		else _datasHistogram.append(QwtIntervalSample(0.,k-1,k));
	}

	if ( _smoothing ) smoothHistogram( _datasHistogram );
	_histogram->setSamples(_datasHistogram);
	updateMaximums();
//	updateMinimums();
	computeMeansAndMedian();
	computeIntervals();
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow &marrow ) {
	_datasHistogram.clear();

	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = _useNextSlice?billon.n_slices-1:billon.n_slices;;
	const int minValue = billon.minValue();
	const int diameter = _marrowAroundDiameter;
	const int radius = diameter/(2.*billon.voxelWidth());
	const int radiusMax = radius+1;
	const qreal squareRadius = qPow(radius,2);
	int i, j, iRadius, iRadiusMax;
	uint k, marrowX, marrowY;
	qreal cumul;

	_datasHistogram.reserve(depth-1);
	//nbPixels = 0;

	QList<int> circleLines;
	circleLines.reserve(2*radius+1);
	for ( j=-radius ; j<radiusMax ; ++j ) {
		circleLines.append(qSqrt(squareRadius-qPow(j,2)));
		//nbPixels += 2*circleLines.last()+1;
	}

	int currentSliceValue, previousSliceValue, nbPixels;
	uint xPos, yPos;
	qreal diff;
	for ( k=1 ; k<depth ; ++k ) {
		const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		marrowX = marrow[k].x;
		marrowY = marrow[k].y;
		cumul = 0.;
		nbPixels = 0;
		for ( j=-radius ; j<radiusMax ; ++j ) {
			iRadius = circleLines[j+radius];
			iRadiusMax = iRadius+1;
			for ( i=-iRadius ; i<iRadiusMax ; ++i ) {
				xPos = marrowX+i;
				yPos = marrowY+j;
				if ( xPos < width && yPos < height ) {
					currentSliceValue = slice.at(yPos,xPos);
					previousSliceValue = prevSlice.at(yPos,xPos);
					if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
						diff = qAbs(currentSliceValue - previousSliceValue);
						if ( (diff > _movementThresholdMin) && (diff < _movementThresholdMax) ) {
							cumul += diff;
						}
						nbPixels++;
					}
				}
			}
		}
		if ( nbPixels > 0 ) _datasHistogram.append(QwtIntervalSample(cumul,k-1,k));
		else _datasHistogram.append(QwtIntervalSample(0.,k-1,k));
	}

	if (_smoothing) smoothHistogram( _datasHistogram );
	_histogram->setSamples(_datasHistogram);
	updateMaximums();
//	updateMinimums();
	computeMeansAndMedian();
	computeIntervals();

	_histogramMaximums->setSamples(_datasMaximums);
}

void SliceHistogram::updateMaximums() {
	_datasMaximums.clear();

	if ( _datasHistogram.size() > 0 ) {
		QList<int> pics;
		const int filterRadius = 15;
		const int max = _datasHistogram.size()-filterRadius;
		double value;
		qDebug() << "Pics primaires :";
		for ( int i=filterRadius ; i<max ; ++i ) {
			value = _datasHistogram.at(i).value;
			if ( (value > _datasHistogram.at(i-1).value) && (value > _datasHistogram.at(i+1).value) &&
				 (value > _datasHistogram.at(i-2).value) && (value > _datasHistogram.at(i+2).value) &&
				 (value > _datasHistogram.at(i-3).value) && (value > _datasHistogram.at(i+3).value) &&
				 (value > _datasHistogram.at(i-4).value) && (value > _datasHistogram.at(i+4).value) &&
				 (value > _datasHistogram.at(i-5).value) && (value > _datasHistogram.at(i+5).value) &&
				 (value > _datasHistogram.at(i-6).value) && (value > _datasHistogram.at(i+6).value) &&
				 (value > _datasHistogram.at(i-7).value) && (value > _datasHistogram.at(i+7).value) &&
				 (value > _datasHistogram.at(i-8).value) && (value > _datasHistogram.at(i+8).value) ) {
				pics.append(i);
				_datasMaximums.append(_datasHistogram.at(i));
				i+=filterRadius-1;
				qDebug() << i;
			}
		}
	}

	_histogramMaximums->setSamples(_datasMaximums);
}

void SliceHistogram::updateMinimums() {
	_datasMinimums.clear();

	if ( _datasHistogram.size() > 0 ) {
		QList<int> pics;
		const int filterRadius = 5;
		const int max = _datasHistogram.size()-filterRadius;
		double value;
		qDebug() << "Pics primaires :";
		for ( int i=filterRadius ; i<max ; ++i ) {
			value = _datasHistogram.at(i).value;
			if ( (value < _datasHistogram.at(i-1).value) && (value < _datasHistogram.at(i+1).value) &&
				 (value < _datasHistogram.at(i-2).value) && (value < _datasHistogram.at(i+2).value) &&
				 (value < _datasHistogram.at(i-3).value) && (value < _datasHistogram.at(i+3).value) &&
				 (value < _datasHistogram.at(i-4).value) && (value < _datasHistogram.at(i+4).value) &&
				 (value < _datasHistogram.at(i-5).value) && (value < _datasHistogram.at(i+5).value) ) {
				pics.append(i);
				_datasMinimums.append(_datasHistogram.at(i));
				i+=filterRadius-1;
				qDebug() << i;
			}
		}
	}

	_histogramMinimums->setSamples(_datasMinimums);
}

namespace {
	inline qreal firstdDerivated( const QVector< QwtIntervalSample > &histogramDatas, const int &index ) {
		return histogramDatas.at(index).value - histogramDatas.at(qAbs(index-1)).value;
	}
}

void SliceHistogram::smoothHistogram( QVector< QwtIntervalSample > &histogramDatas ) {
	qreal veryOldValue = histogramDatas.at(0).value;
	qreal oldValue = histogramDatas.at(1).value;
	qreal temporaryOldValue;
	for ( int i=2 ; i<histogramDatas.size()-2 ; ++i ) {
		temporaryOldValue = histogramDatas.at(i).value;
		histogramDatas[i].value = (veryOldValue + oldValue + temporaryOldValue + histogramDatas.at(i+1).value + histogramDatas.at(i+2).value)/5.;
		//histogramDatas[i].value = (histogramDatas.at(i-2).value, histogramDatas.at(i-1).value + histogramDatas.at(i).value + histogramDatas.at(i+1).value + histogramDatas.at(i+2).value)/5.;
		veryOldValue = oldValue;
		oldValue = temporaryOldValue;
	}
}

void SliceHistogram::computeIntervals() {
	_datasBranchesAreaToDrawing.clear();
	_datasBranchesRealAreas.clear();
	int nbMaximums = this->nbMaximums();
	if ( nbMaximums > 0 ) {
		const int sizeOfHistogram = _datasHistogram.size();
		int cursorMax, cursorMin;
		qreal derivated;
		if ( _intervalType != HistogramIntervalType::FROM_EDGE ) {
			qreal limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_dataMedian;
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				derivated = firstdDerivated(_datasHistogram,cursorMin);
				while ( cursorMin > 0 && (_datasHistogram.at(cursorMin).value > limit || derivated > 0.) ) {
					_datasBranchesAreaToDrawing.append(_datasHistogram.at(cursorMin));
					cursorMin--;
					if ( cursorMin > 0 ) derivated = firstdDerivated(_datasHistogram,cursorMin);
				}
				cursorMin++;

				cursorMax = sliceOfIemeMaximum(i)+1;
				derivated = firstdDerivated(_datasHistogram,cursorMax);
				while ( cursorMax < sizeOfHistogram && (_datasHistogram.at(cursorMax).value > limit || derivated < 0.) ) {
					_datasBranchesAreaToDrawing.append(_datasHistogram.at(cursorMax));
					cursorMax++;
					if ( cursorMax < sizeOfHistogram ) derivated = firstdDerivated(_datasHistogram,cursorMax);
				}
				cursorMax--;

				if ( (cursorMax-cursorMin) > _minimumIntervalWidth ) {
					_datasBranchesRealAreas.append(QwtIntervalSample(1.,cursorMin,cursorMax));
				}
				else {
					_datasMaximums.remove(i);
					nbMaximums--;
					i--;
				}
			}
		}
		else {
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				derivated = firstdDerivated(_datasHistogram,cursorMin);
				while ( cursorMin > 0 && derivated > 0. ) {
					_datasBranchesAreaToDrawing.append(_datasHistogram.at(cursorMin));
					cursorMin--;
					if ( cursorMin > 0 ) derivated = firstdDerivated(_datasHistogram,cursorMin);
				}
				cursorMin++;

				cursorMax = sliceOfIemeMaximum(i)+1;
				derivated = firstdDerivated(_datasHistogram,cursorMax);
				while ( cursorMax < sizeOfHistogram && derivated < 0. ) {
					_datasBranchesAreaToDrawing.append(_datasHistogram.at(cursorMax));
					cursorMax++;
					if ( cursorMax < sizeOfHistogram ) derivated = firstdDerivated(_datasHistogram,cursorMax);
				}
				cursorMax--;

				if ( (cursorMax-cursorMin) > _minimumIntervalWidth ) {
					_datasBranchesRealAreas.append(QwtIntervalSample(1.,cursorMin,cursorMax));
				}
				else {
					_datasMaximums.remove(i);
					nbMaximums--;
					i--;
				}
			}
		}
	}
	_histogramBranchesArea->setSamples(_datasBranchesAreaToDrawing);
}


void SliceHistogram::computeMeansAndMedian() {
	const int nbDatas = _datasHistogram.size();
	qreal xMeans[2] = { 0., nbDatas };
	qreal yMeans[2] = { 0., 0. };
	qreal xMedian[2] = { 0., nbDatas };
	qreal yMedian[2] = { 0., 0. };
	_dataMeans = 0.;
	_dataMedian = 0.;
	if ( nbDatas > 0 ) {
		QVector<qreal> listToSort;
		for ( int i=0 ; i<nbDatas ; ++i ) {
			_dataMeans += _datasHistogram.at(i).value;
			listToSort.append(_datasHistogram.at(i).value);
		}
		_dataMeans /= nbDatas;
		qSort(listToSort);
		_dataMedian = listToSort.at(listToSort.size()/2);
		yMeans[0] = _dataMeans;
		yMeans[1] = _dataMeans;
		yMedian[0] = _dataMedian;
		yMedian[1] = _dataMedian;
	}
	_curveMeans->setSamples(xMeans,yMeans,2);
	_curveMedian->setSamples(xMedian,yMedian,2);
}
