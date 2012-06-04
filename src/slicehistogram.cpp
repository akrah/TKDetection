#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/interval.h"
#include "inc/intervalscomputer.h"

SliceHistogram::SliceHistogram()
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

const QVector<Interval> &SliceHistogram::branchesAreas() const
{
	return _intervals;
}

/*******************************
 * Public setters
 *******************************/

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
	_derivativeThreshold = 0.;
	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow *marrow, const Interval &intensity )
{
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = _useNextSlice?billon.n_slices-1:billon.n_slices;
	const int minValue = intensity.minValue();
	const int maxValue = intensity.maxValue();
	//const int radius = _marrowAroundDiameter/(2.*billon.voxelWidth());
	const int radius = billon.getRestrictedAreaBoudingBoxRadius(marrow,100,-900)*0.5;
	const int radiusMax = radius+1;
	const qreal squareRadius = qPow(radius,2);

	_datas.clear();
	_datas.resize(depth-1);

	QList<int> circleLines;
	if ( marrow != 0 )
	{
		circleLines.reserve(2*radius+1);
		for ( int lineIndex=-radius ; lineIndex<radiusMax ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}
	}

	int i, j, k, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	uint marrowX, marrowY, xPos, yPos;
	qreal cumul, diff;

	const int kLimitMin = 0.02*depth;
	const int kLimitMax = depth-kLimitMin;
	if ( marrow != 0 )
	{
		for ( k=1+kLimitMin ; k<kLimitMax ; ++k )
		{
			const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k-1);
			cumul = 0.;
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
			_datas[k] = cumul;
		}
	}
	else
	{
		const int iHeight = static_cast<int>(height);
		const int iWidth = static_cast<int>(width);
		for ( k=1+kLimitMin ; k<kLimitMax ; ++k )
		{
			const arma::Slice &slice = _useNextSlice?billon.slice(k+1):billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k-1);
			cumul = 0.;
			for ( j=0 ; j<iHeight ; ++j )
			{
				for ( i=0 ; i<iWidth ; ++i )
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
			_datas[k] = cumul;
		}
	}

	if ( _smoothing == SmoothingType::GAUSSIAN ) _datas = IntervalsComputer::gaussianSmoothing( _datas, DEFAULT_MASK_RADIUS, false );
	else if ( _smoothing == SmoothingType::MEANS ) _datas = IntervalsComputer::meansSmoothing( _datas, DEFAULT_MASK_RADIUS, false );
	_derivativeThreshold = IntervalsComputer::minimumThresholdPercentage( _datas, _derivativePercentage );
	_maximums = IntervalsComputer::maximumsComputing( _datas, _derivativeThreshold, _maximumsNeighborhood, false );
	_intervals = IntervalsComputer::intervalsComputing( _datas, _maximums, _derivativeThreshold, _minimumIntervalWidth, false );

	computeValues();
	computeMaximums();
	computeIntervals();

	const qreal x[] = { 0., depth };
	const qreal y[] = { _derivativeThreshold, _derivativeThreshold };
	_curvePercentage.setSamples(x,y,2);
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
