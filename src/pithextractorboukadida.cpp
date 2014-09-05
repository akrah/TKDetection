#include "inc/pithextractorboukadida.h"

#include "inc/billon.h"
#include "inc/coordinate.h"
#include "inc/globalfunctions.h"

PithExtractorBoukadida::PithExtractorBoukadida( const int &subWindowWidth, const int &subWindowHeight, const qreal &pithShift, const uint &smoothingRadius,
												const qreal &minWoodPercentage, const Interval<int> &intensityInterval,
												const bool &ascendingOrder, const TKD::ExtrapolationType &extrapolationType,
												const qreal &firstValidSliceToExtrapolate, const qreal &lastValidSliceToExtrapolate ) :
	_subWindowWidth(subWindowWidth), _subWindowHeight(subWindowHeight), _pithShift(pithShift), _smoothingRadius(smoothingRadius),
	_minWoodPercentage(minWoodPercentage), _intensityInterval(intensityInterval), _ascendingOrder(ascendingOrder),
	_extrapolation(extrapolationType), _validSlices(0,0),
	_firstValidSliceToExtrapolate(firstValidSliceToExtrapolate), _lastValidSliceToExtrapolate(lastValidSliceToExtrapolate)
{
}

PithExtractorBoukadida::~PithExtractorBoukadida()
{
}

int PithExtractorBoukadida::subWindowWidth() const
{
	return _subWindowWidth;
}

int PithExtractorBoukadida::subWindowHeight() const
{
	return _subWindowHeight;
}

qreal PithExtractorBoukadida::pithShift() const
{
	return _pithShift;
}

uint PithExtractorBoukadida::smoothingRadius() const
{
	return _smoothingRadius;
}

qreal PithExtractorBoukadida::minWoodPercentage() const
{
	return _minWoodPercentage;
}

Interval<int> PithExtractorBoukadida::intensityInterval() const
{
	return _intensityInterval;
}

bool PithExtractorBoukadida::ascendingOrder() const
{
	return _ascendingOrder;
}

TKD::ExtrapolationType PithExtractorBoukadida::extrapolation() const
{
	return _extrapolation;
}

const Interval<uint> & PithExtractorBoukadida::validSlices() const
{
	return _validSlices;
}

const uint &PithExtractorBoukadida::firstValidSlicesToExtrapolate() const
{
	return _firstValidSliceToExtrapolate;
}

const uint &PithExtractorBoukadida::lastValidSlicesToExtrapolate() const
{
	return _lastValidSliceToExtrapolate;
}

void PithExtractorBoukadida::setSubWindowWidth( const int &width )
{
	_subWindowWidth = width;
}

void PithExtractorBoukadida::setSubWindowHeight( const int &height )
{
	_subWindowHeight = height;
}

void PithExtractorBoukadida::setPithShift( const qreal & shift )
{
	_pithShift = shift;
}

void PithExtractorBoukadida::setSmoothingRadius( const uint &radius )
{
	_smoothingRadius = radius;
}

void PithExtractorBoukadida::setMinWoodPercentage( const qreal &percentage )
{
	_minWoodPercentage = percentage;
}

void PithExtractorBoukadida::setIntensityInterval( const Interval<int> &interval )
{
	_intensityInterval = interval;
}

void PithExtractorBoukadida::setAscendingOrder( const bool &order )
{
	_ascendingOrder = order;
}

void PithExtractorBoukadida::setExtrapolation( const TKD::ExtrapolationType &extrapolationType )
{
	_extrapolation = extrapolationType;
}

void PithExtractorBoukadida::setFirstValidSlicesToExtrapolate( const uint &percentOfSlices )
{
	_firstValidSliceToExtrapolate = percentOfSlices;
}

void PithExtractorBoukadida::setLastValidSlicesToExtrapolate( const uint &percentOfSlices )
{
	_lastValidSliceToExtrapolate = percentOfSlices;
}

void PithExtractorBoukadida::process( Billon &billon, const bool &adaptativeWidth )
{
	const int widthMinusOne = billon.n_cols-1;
	const int heightMinusOne = billon.n_rows-1;
	const int &depth = billon.n_slices;

	const rCoord2D voxelDims(billon.voxelWidth(),billon.voxelHeight());
	const qreal &xDim = voxelDims.x;
	const qreal &yDim = voxelDims.y;

	const int semiSubWindowWidth = qFloor(_subWindowWidth/(2.*xDim));
	const int semiSubWindowHeight = qFloor(_subWindowHeight/(2.*yDim));
	const int kIncrement = (_ascendingOrder?1:-1);

	Pith &pith = billon._pith;

	int k;

	pith.clear();
	pith.resize(depth);

	QVector<qreal> nbLineByMaxRatio( depth, 0. );
	QVector<qreal> backgroundProportions( depth, 1. );

	qDebug() << "Step 1] Copie du billon";
	Billon billonFillBackground(billon);

	qDebug() << "Step 2] Fill billon background and compute background proportions";
	fillBillonBackground( billonFillBackground, backgroundProportions, _intensityInterval, adaptativeWidth );

	qDebug() << "Step 3] Detect interval of valid slices";
	detectValidSliceInterval( backgroundProportions );
	const int &firstValidSliceIndex = _validSlices.min();
	const int &lastValidSliceIndex = _validSlices.max();
	const int firstSliceOrdered = _ascendingOrder?firstValidSliceIndex:lastValidSliceIndex;
	qDebug() << "[ " << firstValidSliceIndex << ", " << lastValidSliceIndex << " ]";

	if ( _validSlices.size() < _smoothingRadius )
	{
		qDebug() << "   => No valid slices, detection stopped";
		return;
	}

	// Calcul de la moelle sur la première coupe valide
	qDebug() << "Step 4] Hough transform on first valid slice";
	pith[firstSliceOrdered] = transHough( billonFillBackground.slice(firstSliceOrdered), nbLineByMaxRatio[firstSliceOrdered],
										  voxelDims, adaptativeWidth?firstSliceOrdered/static_cast<qreal>(depth):1.0 );

	// Calcul de la moelle sur les coupes suivantes
	qDebug() <<"Step 5] Hough transform on next valid slices";
	rCoord2D currentPithCoord;
	iCoord2D subWindowStart, subWindowEnd;
	for ( k=firstSliceOrdered+kIncrement ; k<=lastValidSliceIndex && k>=firstValidSliceIndex ; k += kIncrement )
	{
		qDebug() << k ;
		const Slice &currentSlice = billonFillBackground.slice(k);
		const rCoord2D &previousPith = pith[k-kIncrement];

		subWindowStart.x = qMax(qFloor(previousPith.x-semiSubWindowWidth),0);
		subWindowEnd.x = qMin(qFloor(previousPith.x+semiSubWindowWidth),widthMinusOne);
		subWindowStart.y = qMax(qFloor(previousPith.y-semiSubWindowHeight),0);
		subWindowEnd.y = qMin(qFloor(previousPith.y+semiSubWindowHeight),heightMinusOne);
		currentPithCoord = transHough( currentSlice.submat( subWindowStart.y, subWindowStart.x, subWindowEnd.y, subWindowEnd.x ), nbLineByMaxRatio[k],
									   voxelDims, 1.0 )
						   + subWindowStart;

		//if ( currentPithCoord.euclideanDistance(previousPith) > _pithShift )
		if ( qSqrt( qPow((currentPithCoord.x-previousPith.x)*xDim,2) + qPow((currentPithCoord.y-previousPith.y)*yDim,2) ) > _pithShift )
		{
			qDebug() << "...  ";
			currentPithCoord = transHough( currentSlice, nbLineByMaxRatio[k], voxelDims, adaptativeWidth?k/static_cast<qreal>(depth):1.0 );
		}

		pith[k] = currentPithCoord;
	}

	// Interpolation des coupes valides
	qDebug() << "Step 6] Interpolation of valid slices";
	interpolation( pith, nbLineByMaxRatio, _validSlices );

	// Lissage
	qDebug() << "Step 7] Smoothing of valid slices";
	TKD::meanSmoothing<rCoord2D>( pith.begin()+firstValidSliceIndex, pith.begin()+lastValidSliceIndex, _smoothingRadius, false );

	// Extrapolation des coupes invalides
	qDebug() << "Step 8] Extrapolation of unvalid slices";

	const int slopeDistance = 3;

	const int firstValidSliceIndexToExtrapolate = firstValidSliceIndex+(lastValidSliceIndex-firstValidSliceIndex)*_firstValidSliceToExtrapolate/100.;
	const int lastValidSliceIndexToExtrapolate = lastValidSliceIndex-(lastValidSliceIndex-firstValidSliceIndex)*_lastValidSliceToExtrapolate/100.;

	const rCoord2D firstValidCoord = pith[firstValidSliceIndexToExtrapolate];
	const rCoord2D lastValidCoord = pith[lastValidSliceIndexToExtrapolate];

	rCoord2D firstValidCoordSlope = (firstValidCoord - pith[firstValidSliceIndexToExtrapolate+slopeDistance])/static_cast<qreal>(slopeDistance);
	// firstValidCoordSlope.x = ((widthMinusOne/2.)-firstValidCoord.x)/static_cast<qreal>(firstValidSliceIndexToExtrapolate);
	const rCoord2D lastValidCoordSlope = (lastValidCoord - pith[lastValidSliceIndexToExtrapolate-slopeDistance])/static_cast<qreal>(slopeDistance);

	switch (_extrapolation)
	{
		case TKD::LINEAR:
			qDebug() << "  Linear extrapolation";
			for ( k=firstValidSliceIndexToExtrapolate-1 ; k>=0 ; --k )
			{
				pith[k] = firstValidCoord;
			}
			for ( k=lastValidSliceIndexToExtrapolate+1 ; k<depth ; ++k )
			{
				pith[k] = lastValidCoord;
			}
			break;
		case TKD::SLOPE_DIRECTION:
			qDebug() <<  "  In slope direction extrapolation";
			for ( k=firstValidSliceIndexToExtrapolate-1 ; k>=0 ; --k )
			{
				pith[k] = pith[k+1] + firstValidCoordSlope;
				pith[k].x = qMin(qMax(pith[k].x,0.),static_cast<qreal>(widthMinusOne));
				pith[k].y = qMin(qMax(pith[k].y,0.),static_cast<qreal>(heightMinusOne));
			}
			for ( k=lastValidSliceIndexToExtrapolate+1 ; k<depth ; ++k )
			{
				pith[k] = pith[k-1] + lastValidCoordSlope;
				pith[k].x = qMin(qMax(pith[k].x,0.),static_cast<qreal>(widthMinusOne));
				pith[k].y = qMin(qMax(pith[k].y,0.),static_cast<qreal>(heightMinusOne));
			}
			break;
		case TKD::NO_EXTRAPOLATION:
		default:
			qDebug() << "  No extrapolation";
			break;
	}
}


uiCoord2D PithExtractorBoukadida::transHough( const Slice &slice, qreal &lineOnMaxRatio, const rCoord2D &voxelDims, const qreal &adaptativeWidthCoeff ) const
{
	const int &width = slice.n_cols;
	const int &height = slice.n_rows;

	const int semiWidth = qFloor(width/2.);
	const int semiAdaptativeWidth = qFloor(semiWidth*adaptativeWidthCoeff);
	const int minX = qMax(semiWidth-semiAdaptativeWidth,0);
	const int maxX = qMin(semiWidth+semiAdaptativeWidth,width-1);

	lineOnMaxRatio = 0.;

	if ( semiAdaptativeWidth<1 )
		return uiCoord2D(semiWidth,qFloor(height/2.));

	// Création de la matrice d'accumulation à partir des filtres de Sobel
	arma::Mat<qreal> orientations( height, maxX-minX+1, arma::fill::zeros );
	arma::Mat<int> accuSlice( height, maxX-minX+1, arma::fill::zeros );
	uint nbContourPoints = accumulation( slice.cols(minX,maxX), orientations, accuSlice, voxelDims );

	// Valeur et coordonnée du maximum de accuSlice
	uiCoord2D pithCoord(width/2,height/2);
	if (nbContourPoints)
	{
		lineOnMaxRatio = accuSlice.max(pithCoord.y,pithCoord.x)/static_cast<qreal>(nbContourPoints);
		pithCoord.x += minX;
	}

	return pithCoord;
}

uint PithExtractorBoukadida::accumulation( const Slice &slice, arma::Mat<qreal> & orientations, arma::Mat<int> &accuSlice,
										   const rCoord2D &voxelDims ) const
{
	const uint widthMinusOne = slice.n_cols-1;
	const uint heightMinusOne = slice.n_rows-1;

	const qreal &xDim = voxelDims.x;
	const qreal &yDim = voxelDims.y;
	const qreal voxelRatio = qPow(xDim/yDim,2);

	arma::Col<qreal> sobelNormVec((widthMinusOne-1)*(heightMinusOne-1));
	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

	uint i, j, nbNegativeNorm;
	qreal sobelX, sobelY, norm;

	nbNegativeNorm = 0;
	for ( i=1 ; i<widthMinusOne ; ++i )
	{
		for ( j=1 ; j<heightMinusOne ; ++j )
		{
			sobelX = slice( j-1, i-1 ) - slice( j-1, i+1 ) +
					 2* (slice( j, i-1 ) - slice( j, i+1 )) +
					 slice( j+1, i-1 ) - slice( j+1, i+1 );
			sobelY = slice( j+1, i-1 ) - slice( j-1, i-1 ) +
					 2 * (slice( j+1, i ) - slice( j-1, i )) +
					 slice( j+1, i+1 ) - slice( j-1, i+1 );
			orientations(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX*voxelRatio;
			norm = qPow(sobelX,2) + qPow(sobelY,2);
			*sobelNormVecIt++ = norm;
			nbNegativeNorm += qFuzzyIsNull(norm);
		}
	}

	if ( nbNegativeNorm >= sobelNormVec.n_elem-2 )
		return 0;

	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
	const qreal &medianVal = sobelNormSort( (sobelNormSort.n_elem + nbNegativeNorm)*0.4 );

	uint nbContourPoints = 0;
	sobelNormVecIt = sobelNormVec.begin();
	for ( i=1 ; i<widthMinusOne ; ++i )
	{
		for ( j=1 ; j<heightMinusOne ; ++j )
		{
			if ( *sobelNormVecIt++ > medianVal )
			{
				drawLine( accuSlice, uiCoord2D(i,j), -orientations(j,i) );
				nbContourPoints++;
			}
		}
	}

	return nbContourPoints;
}

void PithExtractorBoukadida::drawLine(arma::Mat<int> &slice, const iCoord2D &origin, const qreal &orientation ) const
{
	const int heightMinusOne = slice.n_rows-1;
	const int widthMinusOne = slice.n_cols-1;
	const int originX = origin.x;
	const int originY = origin.y;
	const qreal orientationInv = 1./orientation;

	qreal x, y;

	if ( orientation >= 1. )
	{
		for ( x = originX , y=originY; x<widthMinusOne && y<heightMinusOne ; x += orientationInv, y += 1. )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1; x>0. && y>0. ; x -= orientationInv, y -= 1. )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
	}
	else if ( orientation > 0. )
	{
		for ( x = originX, y=originY ; x<widthMinusOne && y<heightMinusOne ; x += 1., y += orientation )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>0. && y>0. ; x -= 1., y -= orientation )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
	}
	else if ( orientation > -1. )
	{
		for ( x = originX, y=originY ; x<widthMinusOne && y>0. ; x += 1., y += orientation )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>0. && y<heightMinusOne ; x -= 1., y -= orientation )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
	}
	else
	{
		for ( x = originX , y=originY; x>0. && y<heightMinusOne ; x += orientationInv, y += 1. )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1.; x<widthMinusOne && y>0. ; x -= orientationInv, y -= 1. )
		{
			slice(qRound(y),qRound(x)) += 1;
		}
	}
}

void PithExtractorBoukadida::interpolation( Pith &pith, const QVector<qreal> &nbLineByMaxRatio, const Interval<uint> &sliceIntervalToInterpolate ) const
{
	const uint &firstSlice = sliceIntervalToInterpolate.min();
	const uint &lastSlice = sliceIntervalToInterpolate.max();

	QVector<qreal> nbLineByMaxRatioSorting = nbLineByMaxRatio.mid( firstSlice, lastSlice-firstSlice+1 );
	qSort(nbLineByMaxRatioSorting);

	const qreal &quartile1 = nbLineByMaxRatioSorting[ 0.25 * nbLineByMaxRatioSorting.size() ];
	const qreal &quartile3 = nbLineByMaxRatioSorting[ 0.75 * nbLineByMaxRatioSorting.size() ];
	const qreal interpolationThreshold = quartile1 - 0.5 * ( quartile3 - quartile1 );

	uint startSliceIndex, newK, startSliceIndexMinusOne;
	rCoord2D interpolationStep, currentInterpolatePithCoord;
	for ( uint k=firstSlice ; k<=lastSlice ; ++k )
	{
		if ( nbLineByMaxRatio[k] < interpolationThreshold )
		{
			startSliceIndex = k++;
			startSliceIndexMinusOne = startSliceIndex?startSliceIndex-1:0;

			while ( k<=lastSlice && nbLineByMaxRatio[k] < interpolationThreshold ) ++k;
			if ( k>startSliceIndex ) --k;

			qDebug() << "Interpolation [" << startSliceIndex << ", " << k << "]" ;

			interpolationStep = k<lastSlice ? (pith[k+1] - pith[startSliceIndexMinusOne]) / static_cast<qreal>( k+1-startSliceIndexMinusOne )
				: rCoord2D(0,0);

			currentInterpolatePithCoord = interpolationStep + pith[startSliceIndexMinusOne];

			for ( newK = startSliceIndex ; newK <= k ; ++newK, currentInterpolatePithCoord += interpolationStep )
			{
				pith[newK] = currentInterpolatePithCoord;
			}
		}
	}
}

void PithExtractorBoukadida::fillBillonBackground( Billon &billonToFill, QVector<qreal> &backgroundProportions,
												   const Interval<int> &intensityInterval, const bool &adaptativeWidth ) const
{
	const int &width = billonToFill.n_cols;
	const int &height = billonToFill.n_rows;
	const int &nbSlices = billonToFill.n_slices;
	const int &minIntensity = intensityInterval.min();
	const int &maxIntensity = intensityInterval.max();

	const int semiWidth = qFloor(width/2.);

	Slice::col_iterator begin, end;
	int k, semiAdaptativeWidth;
	int iMin, iMax;
	qreal adaptativeWidthCoeff, currentProp;
	__billon_type__ val;

	iMin = 0;
	iMax = width-1;
	for ( k=0 ; k<nbSlices ; ++k )
	{
		Slice &currentSlice = billonToFill.slice(k);
		if ( adaptativeWidth )
		{
			adaptativeWidthCoeff = k/static_cast<qreal>(nbSlices);
			semiAdaptativeWidth = qRound(semiWidth*adaptativeWidthCoeff);
			iMin = semiWidth-semiAdaptativeWidth;
			iMax = semiWidth+semiAdaptativeWidth-1;
			if ( semiAdaptativeWidth<2 ) continue;
		}
		begin = currentSlice.begin_col(iMin);
		end = currentSlice.end_col(iMax);
		currentProp = 0.;
		while ( begin != end )
		{
			val = qMax( qMin( *begin, maxIntensity ), minIntensity );
			currentProp += (val == minIntensity || val == maxIntensity);
			*begin++ = val;
		}
		backgroundProportions[k] = currentProp / static_cast<qreal>(height*(iMax-iMin+1));
	}
}

void PithExtractorBoukadida::detectValidSliceInterval( const QVector<qreal> &backgroundProportions )
{
	const uint &nbSlices = backgroundProportions.size();
	const qreal backgroundPercentage = (100.-_minWoodPercentage)/100.;
	uint sliceIndex;;

	sliceIndex = 0;
	while ( sliceIndex<nbSlices && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex++;
	_validSlices.setMin(qMin(sliceIndex,nbSlices-1));

	const uint &minValid = _validSlices.min();
	sliceIndex = nbSlices-1;
	while ( sliceIndex>minValid && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex--;
	_validSlices.setMax(sliceIndex);
}
