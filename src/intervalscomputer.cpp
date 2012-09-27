#include "inc/intervalscomputer.h"

#include "inc/global.h"

namespace IntervalsComputer {

	namespace {
		QVector<qreal> smoothing( const QVector<qreal> &hist, const QVector<qreal> &weights, bool loop = false );
		qreal weightedAccumulation( QVector<qreal>::const_iterator valBegin, QVector<qreal>::const_iterator valEnd, QVector<qreal>::const_iterator weightBegin );
	}

	QVector<qreal> meansSmoothing( const QVector<qreal> &hist, int maskRadius, bool loop ) {
		const int maskWidth = 2*maskRadius+1;
		QVector<qreal> meansWeight(maskWidth,1./static_cast<qreal>(maskWidth));
		return smoothing(hist,meansWeight,loop);
	}

	QVector<qreal> gaussianSmoothing( const QVector<qreal> &hist, int maskRadius, bool loop ) {
		QVector<qreal> gaussianWeight;
		const qreal increment = 3./static_cast<qreal>(maskRadius);
		for ( qreal x=-maskRadius*increment ; x<(maskRadius+1)*increment ; x+=increment ) {
			gaussianWeight.append(increment*GAUSSIAN(x));
		}
		return smoothing(hist,gaussianWeight,loop);
	}

	QVector<int> maximumsComputing( const QVector<qreal> &hist, qreal minimumThreshold, int minimumWidthOfNeighborhood, bool loop )
	{
		const int nbSectors = hist.size();
		QVector<int> maximums;
		if ( nbSectors > 0 )
		{
			qreal value;
			int i, cursor;
			bool isMax;

			QVector<qreal> old;
			if ( loop )
			{
				for ( i=nbSectors-minimumWidthOfNeighborhood ; i<nbSectors ; ++i )
				{
					old << hist[i];
				}
				old << hist;
				for ( i=0 ; i<minimumWidthOfNeighborhood ; ++i )
				{
					old << hist[i];
				}
			}
			else
			{
				for ( i=nbSectors-minimumWidthOfNeighborhood ; i<nbSectors ; ++i )
				{
					old << hist[0];
				}
				old << hist;
				for ( i=0 ; i<minimumWidthOfNeighborhood ; ++i )
				{
					old << hist[nbSectors-1];
				}
			}

			const int end = nbSectors+minimumWidthOfNeighborhood;
			for ( i=minimumWidthOfNeighborhood ; i<end ; ++i )
			{
				value = old[i];
				if ( value > minimumThreshold )
				{
					cursor = 1;
					do
					{
						isMax = ( (value > old[i-cursor]) && (value > old[i+cursor]) );
						cursor++;
					}
					while ( isMax && cursor<=minimumWidthOfNeighborhood );
					if ( isMax )
					{
						maximums.append(i-minimumWidthOfNeighborhood); i+=(minimumWidthOfNeighborhood-1);
					}
				}
			}
		}
		return maximums;
	}

	QVector<Interval> intervalsComputing( const QVector<qreal> &hist, const QVector<int> &maximums, int derivativePercentage, int minimumWidthOfIntervals, bool loop )
	{
		QVector<Interval> intervals;
		if ( !maximums.isEmpty() )
		{
			const int nbSectors = hist.size();
			const int nbMaximums = maximums.size();
			int cursorMax, cursorMin, derivativeThreshold;
			bool fusionLast, fusionFirst;
			cursorMax = -1;
			for ( int i=0 ; i<nbMaximums ; ++i )
			{
				cursorMin = maximums[i];
				derivativeThreshold = hist[cursorMin]*derivativePercentage/100.;
				while ( hist[cursorMin] > derivativeThreshold )
				{
					cursorMin--;
					if ( cursorMin < 0 ) cursorMin = nbSectors-1;
				}
				while ( firstdDerivated(hist,cursorMin,loop) > 0. )
				{
					cursorMin--;
					if ( cursorMin < 0 ) cursorMin = nbSectors-1;
				}

				cursorMax = maximums[i]+1;
				if ( cursorMax == nbSectors ) cursorMax = 0;

				while ( hist[cursorMax] > derivativeThreshold )
				{
					cursorMax++;
					if ( cursorMax == nbSectors ) cursorMax = 0;
				}
				while ( firstdDerivated(hist,cursorMax,loop) < 0. )
				{
					cursorMax++;
					if ( cursorMax == nbSectors ) cursorMax = 0;
				}
				cursorMax--;
				if ( cursorMax<0 ) cursorMax = nbSectors-1;

				if ( cursorMax>cursorMin && (cursorMax-cursorMin) >= minimumWidthOfIntervals )
				{
					if ( intervals.isEmpty() || intervals.last().maxValue() <= cursorMin )
					{
						intervals.append(Interval(cursorMin,cursorMax));
					}
					else if ( intervals.last().isValid() )
					{
						intervals.last().setMin( qMin(intervals.last().minValue(), cursorMin) );
						intervals.last().setMax( qMax(intervals.last().maxValue(), cursorMax) );
						if ( intervals.size() > 1 )
						{
							Interval &previousOfLast = intervals[intervals.size()-2];
							if ( previousOfLast.maxValue() > intervals.last().minValue() )
							{
								previousOfLast.setMin( qMin(previousOfLast.minValue(), intervals.last().minValue()) );
								previousOfLast.setMax( qMax(previousOfLast.maxValue(), intervals.last().maxValue()) );
								intervals.pop_back();
							}
						}
						cursorMin = intervals.last().minValue();
						cursorMax = intervals.last().maxValue();
					}
					else
					{
						intervals.last().setMax( qMax(intervals.last().maxValue(), cursorMax) );
						cursorMax = intervals.last().maxValue();
					}
				}
				else if ( cursorMax<cursorMin && (nbSectors-cursorMin+cursorMax) >= minimumWidthOfIntervals )
				{
					if ( loop )
					{
						if ( intervals.isEmpty() || (intervals.last().isValid() && intervals.last().maxValue() <= cursorMin && intervals.first().isValid() && intervals.first().minValue() >= cursorMax) )
						{
							intervals.append(Interval(cursorMin,cursorMax));
						}
						else
						{
							fusionLast = false;
							if ( intervals.last().isValid() && intervals.last().maxValue() > cursorMin )
							{
								intervals.last().setMin( qMin(intervals.last().minValue(), cursorMin) );
								intervals.last().setMax(cursorMax);
								fusionLast = true;
							}
							if ( !intervals.last().isValid() )
							{
								intervals.last().setMin( qMin(intervals.last().minValue(), cursorMin) );
								intervals.last().setMax( qMax(intervals.last().maxValue(), cursorMax) );
								fusionLast = true;
							}
							if ( !fusionLast ) intervals.append( Interval(cursorMin, cursorMax) );
							else
							{
								cursorMin = intervals.last().minValue();
								cursorMax = intervals.last().maxValue();
							}
							fusionFirst = false;
							if ( intervals.first().isValid() && intervals.first().minValue() < intervals.last().maxValue() )
							{
								intervals.first().setMin( intervals.last().minValue() );
								intervals.first().setMax( qMax(intervals.first().maxValue(), intervals.last().maxValue()) );
								fusionFirst = true;
							}
							if ( !intervals.first().isValid() )
							{
								intervals.first().setMin( qMin(intervals.first().minValue(), intervals.last().minValue()) );
								intervals.first().setMax( qMax(intervals.first().maxValue(), intervals.last().maxValue()) );
								fusionFirst = true;
							}
							if (fusionFirst)
							{
								intervals.pop_back();
								cursorMin = intervals.first().minValue();
								cursorMax = intervals.first().maxValue();
							}
						}
					}
					else if ( intervals.isEmpty() )
					{
						intervals.append(Interval(0,cursorMax));
						cursorMin = 0;
					}
					else
					{
						intervals.append(Interval(cursorMin,nbSectors-1));
						cursorMax = nbSectors-1;
					}
				}
			}
			if ( !intervals.first().isValid() && intervals.last().isValid() && intervals.first().minValue() < intervals.last().maxValue() )
			{
				intervals.first().setMin( qMin(intervals.first().minValue(),intervals.last().minValue()) );
				intervals.pop_back();
			}
		}
		return intervals;
	}

	void defaultComputingOfIntervals( const QVector<qreal> hist, QVector<qreal> &smoothedHist, QVector<int> &maximums, QVector<Interval> &intervals, bool loop ) {
		smoothedHist = gaussianSmoothing( hist, 2, loop );
		const qreal threshold = minimumThresholdPercentage( hist );
		maximums = maximumsComputing( smoothedHist, threshold, 5, loop );
		intervals = intervalsComputing( smoothedHist, maximums, threshold, 1, loop );
	}

	qreal minimumThresholdPercentage( const QVector<qreal> &hist, const qreal &percentage ) {
		const qreal min = minValue(hist.begin(),hist.end());
		const qreal max = maxValue(hist.begin(),hist.end());
		return (max-min)*percentage+min;
	}

	qreal minValue( QVector<qreal>::const_iterator begin, QVector<qreal>::const_iterator end ) {
		qreal res = 0.;
		while ( begin != end ) {
			res = qMin(res,*begin++);
		}
		return res;
	}

	qreal maxValue( QVector<qreal>::const_iterator begin, QVector<qreal>::const_iterator end ) {
		qreal res = 0.;
		while ( begin != end ) {
			res = qMax(res,*begin++);
		}
		return res;
	}

	qreal firstdDerivated( const QVector<qreal> &hist, const int &index, bool loop ) {
		return index>0 ? hist[index] - hist[index-1] : loop ? hist[0] - hist.last() : hist[1] - hist[0];
	}

	namespace {
		QVector<qreal> smoothing( const QVector<qreal> &hist, const QVector<qreal> &weights, bool loop ) {
			const int maskWidth = weights.size();
			const int maskRadius = (maskWidth-1)/2;
			const int nbSectors = hist.size();
			int i;

			QVector<qreal> old;
			if ( loop ) {
				for ( i=nbSectors-maskRadius ; i<nbSectors ; ++i ) {
					old << hist[i];
				}
				old << hist;
				for ( i=0 ; i<maskRadius ; ++i ) {
					old << hist[i];
				}
			}
			else {
				for ( i=nbSectors-maskRadius ; i<nbSectors ; ++i ) {
					old << hist[0];
				}
				old << hist;
				for ( i=nbSectors-maskRadius ; i<nbSectors ; ++i ) {
					old << hist[nbSectors-1];
				}
			}

			QVector<qreal> smoothedHist;
			QVector<qreal>::const_iterator vectBegin = old.begin();
			QVector<qreal>::const_iterator vectEnd = vectBegin+maskWidth;
			while ( vectEnd != old.end() ) {
				smoothedHist.append(weightedAccumulation( vectBegin++, vectEnd++, weights.begin() ));
			}

			return smoothedHist;
		}

		qreal weightedAccumulation( QVector<qreal>::const_iterator valBegin, QVector<qreal>::const_iterator valEnd, QVector<qreal>::const_iterator weightBegin ) {
			qreal res = 0.;
			while ( valBegin != valEnd ) {
				res += (*valBegin++)*(*weightBegin++);
			}
			return res;
		}
	}

}
