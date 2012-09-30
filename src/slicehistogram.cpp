#include "inc/slicehistogram.h"

#include "inc/billon.h"
#include "inc/marrow.h"
#include "inc/interval.h"

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

const QVector< Interval<int> > &SliceHistogram::knotAreas() const
{
	return intervals();
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
	Histogram::clear();

	const QVector<QwtIntervalSample> emptyData(0);
	_histogramData.setSamples(emptyData);
	_histogramMaximums.setSamples(emptyData);
	_histogramIntervals.setSamples(emptyData);
	_curvePercentage.setSamples(QVector<QPointF>(0));
}

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow *marrow, const Interval<int> &intensity, const Interval<int> &motionInterval,
										 const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum,
										 const int &neighborhoodOfMaximums, const int &derivativesPercentage, const int &minimumWidthOfIntervals,
										 const int &borderPercentageToCut, const int &radiusAroundPith )
{
	const uint width = billon.n_cols;
	const uint height = billon.n_rows;
	const uint depth = billon.n_slices;
	const int minValue = intensity.min();
	const int maxValue = intensity.max();
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	clear();
	this->resize(depth-1);

	QList<int> circleLines;
	if ( marrow != 0 )
	{
		circleLines.reserve(2*radiusAroundPith+1);
		for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
		{
			circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
		}
	}

	int i, j, k, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	uint marrowX, marrowY, xPos, yPos;
	qreal cumul, diff;

	const int kLimitMin = borderPercentageToCut*depth/100.;
	const int kLimitMax = depth-kLimitMin;
	if ( marrow != 0 )
	{
		for ( k=kLimitMin ; k<kLimitMax ; ++k )
		{
			const arma::Slice &currentSlice = billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k>0?k-1:k+1);
			cumul = 0.;
			marrowX = marrow->at(k).x;
			marrowY = marrow->at(k).y;
			for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
			{
				iRadius = circleLines[j+radiusAroundPith];
				iRadiusMax = iRadius+1;
				for ( i=-iRadius ; i<iRadiusMax ; ++i )
				{
					xPos = marrowX+i;
					yPos = marrowY+j;
					if ( xPos < width && yPos < height )
					{
						currentSliceValue = currentSlice.at(yPos,xPos);
						previousSliceValue = prevSlice.at(yPos,xPos);
						if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
						{
							diff = qAbs(RESTRICT_TO(minValue,currentSliceValue,maxValue) - RESTRICT_TO(minValue,previousSliceValue,maxValue));
							if ( motionInterval.containsClosed(diff) )
							{
								cumul += diff;
							}
						}
					}
				}
			}
			(*this)[k] = cumul;
		}
	}
	else
	{
		const int iHeight = static_cast<int>(height);
		const int iWidth = static_cast<int>(width);
		for ( k=kLimitMin ; k<kLimitMax ; ++k )
		{
			const arma::Slice &currentSlice = billon.slice(k);
			const arma::Slice &prevSlice = billon.slice(k>0?k-1:k+1);
			cumul = 0.;
			for ( j=0 ; j<iHeight ; ++j )
			{
				for ( i=0 ; i<iWidth ; ++i )
				{
					currentSliceValue = currentSlice.at(j,i);
					previousSliceValue = prevSlice.at(j,i);
					if ( (currentSliceValue > minValue) && (previousSliceValue > minValue) )
					{
						diff = qAbs(RESTRICT_TO(minValue,currentSliceValue,maxValue) - RESTRICT_TO(minValue,previousSliceValue,maxValue));
						if ( motionInterval.containsClosed(diff) )
						{
							cumul += diff;
						}
					}
				}
			}
			(*this)[k] = cumul;
		}
	}

	computeAll( smoothingRadius, minimumHeightPercentageOfMaximum, neighborhoodOfMaximums, derivativesPercentage, minimumWidthOfIntervals, false );

	computeValues();
	computeMaximums();
	computeIntervals();

	const qreal derivativeThreshold = thresholdOfMaximums( minimumHeightPercentageOfMaximum );
	const qreal x[] = { 0., depth };
	const qreal y[] = { derivativeThreshold, derivativeThreshold };
	_curvePercentage.setSamples(x,y,2);
}

void SliceHistogram::computeValues()
{
	QVector<QwtIntervalSample> datasHistogram;
	if ( this->size() > 0 )
	{
		datasHistogram.reserve(this->size());
		qreal sliceValue;
		int i=0;
		QVector<qreal>::ConstIterator begin = this->begin();
		const QVector<qreal>::ConstIterator end = this->end();
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
			datasMaximums.append(QwtIntervalSample(at(slice),slice,slice+1));
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
		QVector< Interval<int> >::ConstIterator begin = _intervals.begin();
		const QVector< Interval<int> >::ConstIterator end = _intervals.end();
		qDebug() << "Intervalles de nœuds :";
		while ( begin != end )
		{
			min = (*begin).min();
			max = (*begin++).max();
			for ( int i=min ; i<max ; ++i )
			{
				dataIntervals.append(QwtIntervalSample(at(i),i,i+1));
			}
			qDebug() << "  [ " << min << ", " << max << " ] avec largeur = " << max-min;
		}
	}
	_histogramIntervals.setSamples(dataIntervals);
}
