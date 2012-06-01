#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/interval.h"
#include "inc/intervalscomputer.h"

SliceHistogram::SliceHistogram() : _dataPercentage(0.), _marrowAroundDiameter(100), _minimumIntervalWidth(DEFAULT_MINIMUM_WIDTH_OF_INTERVALS), _movementThresholdMin(MINIMUM_Z_MOTION),
	_movementThresholdMax(MAXIMUM_Z_MOTION), _smoothing(true), _useNextSlice(true), _maximumsNeighborhood(DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD)
{
	_histogramMaximums.setBrush(Qt::green);
	_histogramMaximums.setPen(QPen(Qt::green));

	_histogramIntervals.setBrush(Qt::blue);
	_histogramIntervals.setPen(QPen(Qt::blue));

	_curvePercentage.setPen(QPen(Qt::red));
}

SliceHistogram::~SliceHistogram()
{
}

/*******************************
 * Public getters
 *******************************/

qreal SliceHistogram::value( const int &index ) const
{
	qreal res = 0.;
	if ( index > -1 && _datas.size() > index )
	{
		res = _datas[index];
	}
	return res;
}

int SliceHistogram::nbMaximums() const
{
	return _maximums.size();
}

int SliceHistogram::sliceOfIemeMaximum( const int &maximumIndex ) const
{
	int sliceIndex = 0;
	if ( maximumIndex>-1 && maximumIndex<_maximums.size() )
	{
		sliceIndex = _maximums[maximumIndex];
	}
	return sliceIndex;
}

int SliceHistogram::sliceOfIemeInterval( const int &intervalIndex ) const
{
	int sliceIndex = 0;
	if ( intervalIndex>-1 && intervalIndex<_intervals.size() )
	{
		const Interval interval = _intervals[intervalIndex];
		sliceIndex = (interval.minValue()+interval.maxValue())/2;
	}
	return sliceIndex;
}

int SliceHistogram::marrowAroundDiameter() const
{
	return _marrowAroundDiameter;
}

const QVector<Interval> &SliceHistogram::branchesAreas() const
{
	return _intervals;
}

/*******************************
 * Public setters
 *******************************/
void SliceHistogram::setMarrowAroundDiameter( const int &diameter )
{
	_marrowAroundDiameter = diameter;
}

void SliceHistogram::setMinimumIntervalWidth( const int &width )
{
	_minimumIntervalWidth = width;
}

void SliceHistogram::setMovementThresholdMin( const int &threshold )
{
	_movementThresholdMin = threshold;
}

void SliceHistogram::setMovementThresholdMax( const int &threshold )
{
	_movementThresholdMax = threshold;
}

void SliceHistogram::enableSmoothing( const bool &enable )
{
	_smoothing = enable;
}

void SliceHistogram::useNextSliceInsteadOfCurrentSlice( const bool &enable )
{
	_useNextSlice = enable;
}

void SliceHistogram::setMaximumsNeighborhood( const int &neighborhood )
{
	_maximumsNeighborhood = neighborhood;
}

void SliceHistogram::attach( QwtPlot * const plot )
{
	if ( plot != 0 )
	{
		_histogramData.attach(plot);
		_histogramIntervals.attach(plot);
		_histogramMaximums.attach(plot);
		_curvePercentage.attach(plot);
	}
}

void SliceHistogram::detach()
{
	_histogramData.detach();
	_histogramIntervals.detach();
	_histogramMaximums.detach();
	_curvePercentage.detach();
}

void SliceHistogram::clear()
{
	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_dataPercentage = 0.;
	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow *marrow, const Interval &intensity )
{
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = _useNextSlice?billon.n_slices-1:billon.n_slices;
	const int minValue = intensity.minValue();
	const int maxValue = intensity.maxValue();
	const int diameter = _marrowAroundDiameter;
	const int radius = diameter/(2.*billon.voxelWidth());
	const int radiusMax = radius+1;
	const qreal squareRadius = qPow(radius,2);

	_datas.clear();
	_datas.reserve(depth-1);

	QList<int> circleLines;
	if ( marrow != 0 )
	{
		circleLines.reserve(2*radius+1);
		for ( int lineIndex=-radius ; lineIndex<radiusMax ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}
	}

	int i, j, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	uint k, marrowX, marrowY, xPos, yPos;
	qreal cumul, diff;
	for ( k=1 ; k<depth ; ++k )
	{
		const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
		const arma::Slice &prevSlice = billon.slice(k-1);
		cumul = 0.;
		if ( marrow != 0 )
		{
			marrowX = marrow->at(k).x;
			marrowY = marrow->at(k).y;
			for ( j=-radius ; j<radiusMax ; ++j )
			{
				iRadius = circleLines[j+radius];
				iRadiusMax = iRadius+1;
				for ( i=-iRadius ; i<iRadiusMax ; ++i )
				{
					xPos = marrowX+i;
					yPos = marrowY+j;
					if ( xPos < width && yPos < height )
					{
						currentSliceValue = slice.at(yPos,xPos);
						previousSliceValue = prevSlice.at(yPos,xPos);
						if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
						{
							diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
							if ( (diff >= _movementThresholdMin) && (diff <= _movementThresholdMax) )
							{
								cumul += diff;
							}
						}
					}
				}
			}
		}
		else
		{
			for ( j=0 ; j<static_cast<int>(height) ; ++j )
			{
				for ( i=0 ; i<static_cast<int>(width) ; ++i )
				{
					currentSliceValue = slice.at(j,i);
					previousSliceValue = prevSlice.at(j,i);
					if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
					{
						diff = qAbs(qBound(minValue,currentSliceValue,maxValue) - qBound(minValue,previousSliceValue,maxValue));
						if ( (diff >= _movementThresholdMin) && (diff <= _movementThresholdMax) )
						{
							cumul += diff;
						}
					}
				}
			}
		}
		_datas.append(cumul);
	}

	if ( _smoothing ) _datas = IntervalsComputer::gaussianSmoothing( _datas, DEFAULT_MASK_RADIUS );
	_dataPercentage = IntervalsComputer::minimumThresholdPercentage( _datas );
	const qreal x[] = { 0., depth };
	const qreal y[] = { _dataPercentage, _dataPercentage };
	_curvePercentage.setSamples(x,y,2);
	_maximums = IntervalsComputer::maximumsComputing( _datas, _dataPercentage, _maximumsNeighborhood );
	_intervals = IntervalsComputer::intervalsComputing( _datas, _maximums, _dataPercentage, _minimumIntervalWidth );

	computeValues();
	computeMaximums();
	computeIntervals();
}

void SliceHistogram::computeValues()
{
	QVector<QwtIntervalSample> datasHistogram;
	if ( _datas.size() > 0 )
	{
		datasHistogram.reserve(_datas.size());
		qreal sliceValue;
		int i=0;
		QVector<qreal>::ConstIterator begin = _datas.begin();
		const QVector<qreal>::ConstIterator end = _datas.end();
		while ( begin != end )
		{
			sliceValue = *begin++;
			datasHistogram.append(QwtIntervalSample(sliceValue,i,i+1));
			i++;
		}
	}
	_histogramData.setSamples(datasHistogram);
}

void SliceHistogram::computeMaximums()
{
	QVector<QwtIntervalSample> datasMaximums;
	if ( _maximums.size() > 0 )
	{
		int slice;
		QVector<int>::ConstIterator begin = _maximums.begin();
		const QVector<int>::ConstIterator end = _maximums.end();
		while ( begin != end )
		{
			slice = *begin++;
			datasMaximums.append(QwtIntervalSample(_datas[slice],slice,slice+1));
		}
	}
	_histogramMaximums.setSamples(datasMaximums);
}

void SliceHistogram::computeIntervals()
{
	QVector<QwtIntervalSample> dataIntervals;
	int nbMaximums = _maximums.size();
	if ( nbMaximums > 0 )
	{
		dataIntervals.reserve(nbMaximums*25);
		int min, max;
		QVector<Interval>::ConstIterator begin = _intervals.begin();
		const QVector<Interval>::ConstIterator end = _intervals.end();
		qDebug() << "Intervalles de branches :";
		while ( begin != end )
		{
			min = (*begin).minValue();
			max = (*begin++).maxValue();
			for ( int i=min ; i<max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(_datas[i],i,i+1));
			}
			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
