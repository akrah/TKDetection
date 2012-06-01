#include "inc/intervalscomputer.h"

#include "inc/global.h"

namespace IntervalsComputer {

	namespace {
		QVector<qreal> smoothing( const QVector<qreal> &hist, const QVector<qreal> &weights, bool loop = false );
		qreal weightedAccumulation( QVector<qreal>::ConstIterator valBegin, QVector<qreal>::ConstIterator valEnd, QVector<qreal>::ConstIterator weightBegin );
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

	QVector<int> maximumsComputing( const QVector<qreal> &hist, qreal minimumThreshold, int minimumWidthOfNeighborhood, bool loop ) {
		const int nbSectors = hist.size();
		QVector<int> maximums;
		if ( nbSectors > 0 ) {
			qreal value;
			int i, cursor;
			bool isMax;

			QVector<qreal> old;
			if ( loop ) {
				for ( i=nbSectors-minimumWidthOfNeighborhood ; i<nbSectors ; ++i ) {
					old << hist[i];
				}
				old << hist;
				for ( i=0 ; i<minimumWidthOfNeighborhood ; ++i ) {
					old << hist[i];
				}
			}
			else {
				for ( i=nbSectors-minimumWidthOfNeighborhood ; i<nbSectors ; ++i ) {
					old << hist[0];
				}
				old << hist;
				for ( i=nbSectors-minimumWidthOfNeighborhood ; i<minimumWidthOfNeighborhood ; ++i ) {
					old << hist[nbSectors-1];
				}
			}

			const int end = nbSectors+minimumWidthOfNeighborhood;
			for ( i=minimumWidthOfNeighborhood ; i<end ; ++i ) {
				value = old[i];
				if ( value > minimumThreshold ) {
					cursor = 1;
					do {
						isMax = ( (value > old[i-cursor]) && (value > old[i+cursor]) );
						cursor++;
					}
					while ( isMax && cursor<minimumWidthOfNeighborhood );
					if ( isMax ) {
						maximums.append(i-minimumWidthOfNeighborhood);
					}
				}
			}
		}
		return maximums;
	}

	QVector<Interval> intervalsComputing( const QVector<qreal> &hist, const QVector<int> &maximums, qreal derivativeThreshold, int minimumWidthOfIntervals, bool loop ) {
		QVector<Interval> intervals;
		if ( !maximums.isEmpty() ) {
			const int nbSectors = hist.size();
			const int nbMaximums = maximums.size();
			int cursorMax, cursorMin;
			bool isSupToThreshold;
			for ( int i=0 ; i<nbMaximums ; ++i ) {
				cursorMin = maximums[i];
				// Si c'est le premier intervalle ou que le maximum courant n'est pas compris dans l'intervalle précédent.
				if ( intervals.isEmpty() || intervals.last().minValue() < cursorMin ) {
					// On recherche les bornes min et max des potentielles de l'intervalle contenant le ième maximum
					isSupToThreshold = hist[cursorMin] > derivativeThreshold;
					while ( isSupToThreshold ) {
						cursorMin--;
						if ( cursorMin < 0 ) cursorMin = nbSectors-1;
						isSupToThreshold &= hist[cursorMin] > derivativeThreshold;
					}
					while ( firstdDerivated(hist,cursorMin,loop) > 0. ) {
						cursorMin--;
						if ( cursorMin < 0 ) cursorMin = nbSectors-1;
					}

					cursorMax = maximums[i]+1;
					if ( cursorMax == nbSectors ) cursorMax = 0;
					isSupToThreshold = hist[cursorMax] > derivativeThreshold;
					while ( isSupToThreshold ) {
						cursorMax++;
						if ( cursorMax >= nbSectors ) cursorMax = 0;
						isSupToThreshold &= hist[cursorMax] > derivativeThreshold;
					}
					while ( firstdDerivated(hist,cursorMax,loop) < 0. ) {
						cursorMax++;
						if ( cursorMax >= nbSectors ) cursorMax = 0;
					}
					cursorMax--;
					if ( cursorMax<0 ) cursorMax = nbSectors-1;

					// Si c'est le premier intervalle ou que le maximum courant n'est pas compris dans l'intervalle précédent.
					if ( intervals.isEmpty() || intervals.first().maxValue() != cursorMax ) {
						// Si l'intervalle est plus large que minimumWidthOfIntervals
						if ( cursorMax>cursorMin && qAbs(cursorMax-cursorMin) >= minimumWidthOfIntervals ) {
							intervals.append(Interval(cursorMin,cursorMax));
						}
						else if ( cursorMax<cursorMin && qAbs(cursorMax-(cursorMin-nbSectors)) >= minimumWidthOfIntervals ) {
							if ( loop ) intervals.append(Interval(cursorMin,cursorMax));
							else if ( intervals.isEmpty() ) {
								intervals.append(Interval(0,cursorMax));
							}
							else {
								intervals.append(Interval(cursorMin,nbSectors-1));
							}
						}
					}
					else {
						i=nbMaximums;
					}
				}
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

	qreal minimumThresholdPercentage( const QVector<qreal> &hist ) {
		const qreal min = minValue(hist.begin(),hist.end());
		const qreal max = maxValue(hist.begin(),hist.end());
		return (max-min)*PERCENTAGE_FOR_MAXIMUM_CANDIDATE+min;
	}

	qreal minValue( QVector<qreal>::ConstIterator begin, QVector<qreal>::ConstIterator end ) {
		qreal res = *begin++;
		while ( begin != end ) {
			res = qMin(res,*begin++);
		}
		return res;
	}

	qreal maxValue( QVector<qreal>::ConstIterator begin, QVector<qreal>::ConstIterator end ) {
		qreal res = *begin++;
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
			QVector<qreal>::ConstIterator vectBegin = old.begin();
			QVector<qreal>::ConstIterator vectEnd = vectBegin+maskWidth;
			for ( i=0 ; i<nbSectors ; ++i ) {
				smoothedHist.append(weightedAccumulation( vectBegin++, vectEnd++, weights.begin() ));
			}

			return smoothedHist;
		}

		qreal weightedAccumulation( QVector<qreal>::ConstIterator valBegin, QVector<qreal>::ConstIterator valEnd, QVector<qreal>::ConstIterator weightBegin ) {
			qreal res = 0.;
			while ( valBegin != valEnd ) {
				res += (*valBegin++)*(*weightBegin++);
			}
			return res;
		}
	}

}
