#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/intensityinterval.h"
#include <qwt_plot_histogram.h>
#include <qwt_plot_curve.h>

SliceHistogram::SliceHistogram() : _histogram(new QwtPlotHistogram()), _histogramMaximums(new QwtPlotHistogram()), _histogramBranchesArea(new QwtPlotHistogram()),
	_curveMeans(new QwtPlotCurve()), _dataMeans(0.), _curveMedian(new QwtPlotCurve()), _dataMedian(0.), _curveMeansMedian(new QwtPlotCurve()), _dataMeansMedian(0.), _marrowAroundDiameter(50), _intervalType(HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN),
	_minimumIntervalWidth(10), _movementThresholdMin(100), _movementThresholdMax(200), _smoothing(true), _useNextSlice(true), _maximumsNeighborhood(10)
{
	_histogramMaximums->setBrush(Qt::green);
	_histogramMaximums->setPen(QPen(Qt::green));

	_histogramBranchesArea->setBrush(Qt::blue);
	_histogramBranchesArea->setPen(QPen(Qt::blue));

	_curveMeans->setPen(QPen(Qt::red));
	_curveMedian->setPen(QPen(Qt::red));
	_curveMeansMedian->setPen(QPen(Qt::red));
}

SliceHistogram::~SliceHistogram() {
	if ( _histogram != 0 ) delete _histogram;
	if ( _histogramMaximums != 0 ) delete _histogramMaximums;
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
	int sliceIndex = 0;
	if ( maximumIndex>-1 && maximumIndex<_datasMaximums.size() ) {
		sliceIndex = _datasMaximums.at(maximumIndex).interval.minValue();
	}
	return sliceIndex;
}

int SliceHistogram::sliceOfIemeInterval( const int &intervalIndex ) const {
	int sliceIndex = 0;
	if ( intervalIndex>-1 && intervalIndex<_datasBranchesRealAreas.size() ) {
		const QwtInterval interval = _datasBranchesRealAreas.at(intervalIndex);
		sliceIndex = (interval.minValue()+interval.maxValue())/2;
	}
	return sliceIndex;
}

int SliceHistogram::marrowAroundDiameter() const {
	return _marrowAroundDiameter;
}

const QVector<QwtInterval> & SliceHistogram::branchesAreas() const {
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

void SliceHistogram::useNextSliceInsteadOfCurrentSlice( const bool &enable ) {
	_useNextSlice = enable;
}

void SliceHistogram::setMaximumsNeighborhood( const int &neighborhood ) {
	_maximumsNeighborhood = neighborhood;
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
		if ( _intervalType == HistogramIntervalType::FROM_MEANS && _curveMeans != 0 ) {
			_curveMeans->attach(plot);
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MEDIAN && _curveMedian != 0 ) {
			_curveMedian->attach(plot);
		}
		else if ( _intervalType == HistogramIntervalType::FROM_MIDDLE_OF_MEANS_AND_MEDIAN && _curveMeansMedian != 0 ) {
			_curveMeansMedian->attach(plot);
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
	if ( _curveMeans != 0 ) {
		_curveMeans->detach();
	}
	if ( _curveMedian != 0 ) {
		_curveMedian->detach();
	}
	if ( _curveMeansMedian != 0 ) {
		_curveMeansMedian->detach();
	}
}

void SliceHistogram::clear() {
	_datasHistogram.clear();
	_histogram->setSamples(_datasHistogram);
	_datasMaximums.clear();
	_histogramMaximums->setSamples(_datasMaximums);
	_datasBranchesAreaToDrawing.clear();
	_datasBranchesRealAreas.clear();
	_histogramBranchesArea->setSamples(_datasBranchesAreaToDrawing);
	_dataMeans = 0.;
	_curveMeans->setSamples(QVector<QPointF>());
	_dataMedian = 0.;
	_curveMedian->setSamples(QVector<QPointF>());
	_dataMeansMedian = 0.;
	_curveMeansMedian->setSamples(QVector<QPointF>());
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow *marrow, const IntensityInterval &intensity ) {
	_datasHistogram.clear();

	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = _useNextSlice?billon.n_slices-1:billon.n_slices;
	const int minValue = intensity.min();
	const int maxValue = intensity.max();
	const int diameter = _marrowAroundDiameter;
	const int radius = diameter/(2.*billon.voxelWidth());
	const int radiusMax = radius+1;
	const qreal squareRadius = qPow(radius,2);

	_datasHistogram.reserve(depth-1);
	//nbPixels = 0;

	QList<int> circleLines;
	if ( marrow != 0 ) {
		circleLines.reserve(2*radius+1);
		for ( int lineIndex=-radius ; lineIndex<radiusMax ; ++lineIndex ) {
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
			//nbPixels += 2*circleLines.last()+1;
		}
	}

	int i, j, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	uint k, marrowX, marrowY, xPos, yPos;
	qreal cumul, diff;
	for ( k=1 ; k<depth ; ++k ) {
		const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		cumul = 0.;
		if ( marrow != 0 ) {
			marrowX = marrow->at(k).x;
			marrowY = marrow->at(k).y;
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
							diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
							if ( (diff > _movementThresholdMin) && (diff < _movementThresholdMax) ) {
								cumul += diff;
							}
						}
					}
				}
			}
		}
		else {
			for ( j=0 ; j<static_cast<int>(height) ; ++j ) {
				for ( i=0 ; i<static_cast<int>(width) ; ++i ) {
					currentSliceValue = slice.at(j,i);
					previousSliceValue = prevSlice.at(j,i);
					if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) ) {
						diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
						if ( (diff > _movementThresholdMin) && (diff < _movementThresholdMax) ) {
							cumul += diff;
						}
					}
				}
			}
		}
		_datasHistogram.append(QwtIntervalSample(cumul,k-1,k));
	}

	if (_smoothing) smoothHistogram( _datasHistogram );
	_histogram->setSamples(_datasHistogram);
	computeMaximums();
	computeMeansAndMedian();
	computeIntervals();
}

void SliceHistogram::computeMaximums() {
	_datasMaximums.clear();

	if ( _datasHistogram.size() > 0 ) {
		const int max = _datasHistogram.size()-_maximumsNeighborhood;
		double value;
		int cursor;
		bool isMax;
		qDebug() << "Pics :";
		for ( int i=_maximumsNeighborhood ; i<max ; ++i ) {
			value = _datasHistogram.at(i).value;
			if ( value > 0 ) {
				cursor = 1;
				do {
					isMax = ( (value > _datasHistogram.at(i-cursor).value) && (value > _datasHistogram.at(i+cursor).value) );
					cursor++;
				}
				while ( isMax && cursor<=_maximumsNeighborhood );
				if ( isMax ) {
					_datasMaximums.append(_datasHistogram.at(i));
					i+=_maximumsNeighborhood-1;
					qDebug() << i;
				}
			}
		}
	}

	_histogramMaximums->setSamples(_datasMaximums);
}

namespace {
	inline qreal firstdDerivated( const QVector< QwtIntervalSample > &histogramDatas, const int &index ) {
		return histogramDatas.at(index).value - histogramDatas.at(qAbs(index-1)).value;
	}
}

void SliceHistogram::smoothHistogram( QVector< QwtIntervalSample > &histogramDatas ) {
	qreal veryOldValue = histogramDatas.at(0).value;
	qreal oldValue = histogramDatas.at(1).value;
	qreal currentValue;
	int nbDatas = histogramDatas.size()-2;
	for ( int i=2 ; i<nbDatas ; ++i ) {
		currentValue = histogramDatas.at(i).value;
		histogramDatas[i].value = (veryOldValue + oldValue + currentValue + histogramDatas.at(i+1).value + histogramDatas.at(i+2).value)/5.;
		veryOldValue = oldValue;
		oldValue = currentValue;
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
		QVector<QwtIntervalSample> setOfIntervals;
		if ( _intervalType != HistogramIntervalType::FROM_EDGE ) {
			qreal limit = _intervalType==HistogramIntervalType::FROM_MEANS?_dataMeans:_intervalType==HistogramIntervalType::FROM_MEDIAN?_dataMedian:_dataMeansMedian;
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				if (_datasBranchesRealAreas.size() == 0 || _datasBranchesRealAreas.last().maxValue() < cursorMin ) {
					setOfIntervals.clear();
					derivated = firstdDerivated(_datasHistogram,cursorMin);
					while ( cursorMin > 0 && (_datasHistogram.at(cursorMin).value > limit || derivated > 0.) ) {
						setOfIntervals.append(_datasHistogram.at(cursorMin));
						cursorMin--;
						if ( cursorMin > 0 ) derivated = firstdDerivated(_datasHistogram,cursorMin);
					}
					cursorMin++;

					if (_datasBranchesRealAreas.size() == 0 || _datasBranchesRealAreas.last().minValue() != cursorMin ) {
						cursorMax = sliceOfIemeMaximum(i)+1;
						derivated = firstdDerivated(_datasHistogram,cursorMax);
						while ( cursorMax < sizeOfHistogram && (_datasHistogram.at(cursorMax).value > limit || derivated < 0.) ) {
							setOfIntervals.append(_datasHistogram.at(cursorMax));
							cursorMax++;
							if ( cursorMax < sizeOfHistogram ) derivated = firstdDerivated(_datasHistogram,cursorMax);
						}
						cursorMax--;

						if ( (cursorMax-cursorMin) > _minimumIntervalWidth ) {
							_datasBranchesAreaToDrawing << setOfIntervals;
							_datasBranchesRealAreas.append(QwtInterval(cursorMin,cursorMax));
						}
						else {
							_datasMaximums.remove(i);
							nbMaximums--;
							i--;
						}
					}
				}
			}
		}
		else {
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = sliceOfIemeMaximum(i);
				if (_datasBranchesRealAreas.size() == 0 || _datasBranchesRealAreas.last().maxValue() < cursorMin ) {
					setOfIntervals.clear();
					derivated = firstdDerivated(_datasHistogram,cursorMin);
					while ( cursorMin > 0 && derivated > 0. ) {
						setOfIntervals.append(_datasHistogram.at(cursorMin));
						cursorMin--;
						if ( cursorMin > 0 ) derivated = firstdDerivated(_datasHistogram,cursorMin);
					}
					cursorMin++;

					if (_datasBranchesRealAreas.size() == 0 || _datasBranchesRealAreas.last().minValue() != cursorMin ) {
						cursorMax = sliceOfIemeMaximum(i)+1;
						derivated = firstdDerivated(_datasHistogram,cursorMax);
						while ( cursorMax < sizeOfHistogram && derivated < 0. ) {
							setOfIntervals.append(_datasHistogram.at(cursorMax));
							cursorMax++;
							if ( cursorMax < sizeOfHistogram ) derivated = firstdDerivated(_datasHistogram,cursorMax);
						}
						cursorMax--;

						if ( (cursorMax-cursorMin) > _minimumIntervalWidth ) {
							_datasBranchesAreaToDrawing << setOfIntervals;
							_datasBranchesRealAreas.append(QwtInterval(cursorMin,cursorMax));
						}
						else {
							_datasMaximums.remove(i);
							nbMaximums--;
							i--;
						}
					}
				}
			}
		}
	}
	_histogramBranchesArea->setSamples(_datasBranchesAreaToDrawing);
	_histogramMaximums->setSamples(_datasMaximums);

	qDebug() << "Intervalles de branches :";
	for ( int i=0 ; i<_datasBranchesRealAreas.size() ; ++i ) {
		const QwtInterval &interval = _datasBranchesRealAreas.at(i);
		qDebug() << "  [ " << interval.minValue() << ", " << interval.maxValue() << " ] avec largeur = " << interval.width();
	}
}


void SliceHistogram::computeMeansAndMedian() {
	const int nbDatas = _datasHistogram.size();
	qreal xMeans[2] = { 0., nbDatas };
	qreal yMeans[2] = { 0., 0. };
	qreal xMedian[2] = { 0., nbDatas };
	qreal yMedian[2] = { 0., 0. };
	qreal xMeansMedian[2] = { 0., nbDatas };
	qreal yMeansMedian[2] = { 0., 0. };
	qreal currentValue;
	_dataMeans = 0.;
	_dataMedian = 0.;
	_dataMeansMedian = 0.;
	if ( nbDatas > 0 ) {
		QVector<qreal> listToSort(nbDatas);
		for ( int i=0 ; i<nbDatas ; ++i ) {
			currentValue = _datasHistogram.at(i).value;
			_dataMeans += currentValue;
			listToSort[i] = currentValue;
		}
		_dataMeans /= static_cast<qreal>(nbDatas);
		qSort(listToSort);
		if ( nbDatas % 2 == 0 ) _dataMedian = (listToSort.at((nbDatas/2)-1)+listToSort.at(nbDatas/2))/2.;
		else _dataMedian = listToSort.at((nbDatas+1)/2-1);
		_dataMeansMedian = (_dataMeans+_dataMedian)/2.;

		yMeans[0] = yMeans[1] = _dataMeans;
		yMedian[0] = yMedian[1] = _dataMedian;
		yMeansMedian[0] = yMeansMedian[1] = _dataMeansMedian;
	}
	_curveMeans->setSamples(xMeans,yMeans,2);
	_curveMedian->setSamples(xMedian,yMedian,2);
	_curveMeansMedian->setSamples(xMeansMedian,yMeansMedian,2);
}
