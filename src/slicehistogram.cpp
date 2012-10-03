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

void SliceHistogram::constructHistogram( const Billon &billon, const Marrow &marrow, const Interval<int> &intensity, const Interval<int> &motionInterval,
										 const int &smoothingRadius, const int &minimumHeightPercentageOfMaximum,
										 const int &neighborhoodOfMaximums, const int &derivativesPercentage, const int &minimumWidthOfIntervals,
										 const int &borderPercentageToCut, const int &radiusAroundPith )
{
	const int width = billon.n_cols;
	const int height = billon.n_rows;
	const int depth = billon.n_slices;
	const int radiusMax = radiusAroundPith+1;
	const qreal squareRadius = qPow(radiusAroundPith,2);

	clear();
	this->resize(depth-1);

	QVector<int> circleLines;
	circleLines.reserve(2*radiusAroundPith+1);
	for ( int lineIndex=-radiusAroundPith ; lineIndex<radiusMax ; ++lineIndex )
	{
		circleLines.append(qSqrt(squareRadius-qPow(lineIndex,2)));
	}

	int i, j, k, iRadius, iRadiusMax, currentSliceValue, previousSliceValue;
	iCoord2D currentPos;
	qreal cumul, diff;

	const int kLimitMin = borderPercentageToCut*depth/100.;
	const int kLimitMax = depth-kLimitMin;
	for ( k=kLimitMin ; k<kLimitMax ; ++k )
	{
		const Slice &currentSlice = billon.slice(k);
		const Slice &prevSlice = billon.slice(k>0?k-1:k+1);
		cumul = 0.;
		currentPos.y = marrow.at(k).y-radiusAroundPith;
		for ( j=-radiusAroundPith ; j<radiusMax ; ++j )
		{
			iRadius = circleLines[j+radiusAroundPith];
			iRadiusMax = iRadius+1;
			currentPos.x = marrow.at(k).x-iRadius;
			for ( i=-iRadius ; i<iRadiusMax ; ++i )
			{
				if ( currentPos.x < width && currentPos.y < height )
				{
					currentSliceValue = currentSlice.at(currentPos.y,currentPos.x);
					previousSliceValue = prevSlice.at(currentPos.y,currentPos.x);
					if ( intensity.containsClosed(currentSliceValue) && intensity.containsClosed(previousSliceValue) )
					{
						diff = qAbs(currentSliceValue - previousSliceValue);
						if ( motionInterval.containsClosed(diff) ) cumul += diff;
					}
				}
				currentPos.x++;
			}
			currentPos.y++;
		}
		(*this)[k] = cumul;
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
