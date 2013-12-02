#include "inc/pithextractorboukadida.h"

#include "inc/billon.h"
#include "inc/coordinate.h"
#include "inc/globalfunctions.h"

PithExtractorBoukadida::PithExtractorBoukadida( const int &subWindowWidth, const int &subWindowHeight, const qreal &pithShift, const uint &smoothingRadius ,
												const qreal &minWoodPercentage, const Interval<int> &intensityInterval, const bool &ascendingOrder ) :
	_subWindowWidth(subWindowWidth), _subWindowHeight(subWindowHeight), _pithShift(pithShift), _smoothingRadius(smoothingRadius),
	_minWoodPercentage(minWoodPercentage), _intensityInterval(intensityInterval), _ascendingOrder(ascendingOrder)
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

void PithExtractorBoukadida::process( Billon &billon, const bool &adaptativeWidth ) const
{
	const int &width = billon.n_cols-1;
	const int &height = billon.n_rows-1;
	const int &depth = billon.n_slices;
	const rCoord3D &voxelDims = billon.voxelDims();
	const int semiSubWindowWidth = _subWindowWidth/2;
	const int semiSubWindowHeight = _subWindowHeight/2;
	const int kIncrement = (_ascendingOrder?1:-1);
	Pith &pith = billon._pith;

	int k;

	pith.clear();
	pith.resize(depth);

	QVector<qreal> nbLineByMaxRatio( depth, 0. );
	QVector<qreal> backgroundProportions;

	std::cout << "Step 1] Copie du billon" << std::endl;
	Billon billonFillBackground(billon);

	std::cout << "Step 2] Fill billon background and compute background proportions" << std::endl;
	fillBillonBackground( billonFillBackground, backgroundProportions, _intensityInterval, adaptativeWidth );

	std::cout << "Step 3] Detect interval of valid slices" << std::endl;
	const Interval<uint> validSliceInterval = detectValidSliceInterval( backgroundProportions );
	const int &firstValidSliceIndex = validSliceInterval.min();
	const int &lastValidSliceIndex = validSliceInterval.max();
	const int firstSliceOrdered = _ascendingOrder?firstValidSliceIndex:lastValidSliceIndex;

	// Calcul de la moelle sur la première coupe valide
	std::cout << "Step 4] Hough transform on first valid slice" << std::endl;
	pith[firstSliceOrdered] = transHough( billonFillBackground.slice(firstSliceOrdered), nbLineByMaxRatio[firstSliceOrdered],
										  voxelDims, adaptativeWidth?firstSliceOrdered/static_cast<qreal>(depth):1.0 );

	// Calcul de la moelle sur les coupes suivantes
	std::cout << "Step 5] Hough transform on next valid slices" << std::endl;
	rCoord2D currentPithCoord;
	iCoord2D subWindowStart, subWindowEnd;
	for ( k=firstSliceOrdered+kIncrement ; k<=lastValidSliceIndex && k>=firstValidSliceIndex ; k += kIncrement )
	{
		std::cout << k << "  ";
		const Slice &currentSlice = billonFillBackground.slice(k);
		const rCoord2D &previousPith = pith[k-kIncrement];

		subWindowStart.x = qMax(qFloor(previousPith.x-semiSubWindowWidth),0);
		subWindowEnd.x = qMin(qFloor(previousPith.x+semiSubWindowWidth),width-1);
		subWindowStart.y = qMax(qFloor(previousPith.y-semiSubWindowHeight),0);
		subWindowEnd.y = qMin(qFloor(previousPith.y+semiSubWindowHeight),height-1);
		currentPithCoord = transHough( currentSlice.submat( subWindowStart.y, subWindowStart.x, subWindowEnd.y, subWindowEnd.x ), nbLineByMaxRatio[k],
									   voxelDims, adaptativeWidth?k/static_cast<qreal>(depth):1.0 )
						   + subWindowStart;

		if ( currentPithCoord.euclideanDistance(previousPith) > _pithShift )
		{
			std::cout << "...  ";
			currentPithCoord = transHough( currentSlice, nbLineByMaxRatio[k], voxelDims, adaptativeWidth?k/static_cast<qreal>(depth):1.0 );
		}

		pith[k] = currentPithCoord;
	}
	std::cout << std::endl;

	// Extrapolation des coupes invalides
	std::cout << "Step 6] Extrapolation of unvalid slices" << std::endl;
	const rCoord2D &firstValidCoord = pith[firstValidSliceIndex];
	for ( k=0 ; k<firstValidSliceIndex ; ++k )
	{
		pith[k] = firstValidCoord;
	}
	const rCoord2D &lastValidCoord = pith[lastValidSliceIndex];
	for ( k=lastValidSliceIndex+1 ; k<depth ; ++k )
	{
		pith[k] = lastValidCoord;
	}

	// Interpolation des coupes valides
	std::cout << "Step 7] Interpolation of valid slices" << std::endl;
	interpolation( pith, nbLineByMaxRatio, validSliceInterval );

	// Lissage
	std::cout << "Step 8] Smoothing of valid slices" << std::endl;
	TKD::meanSmoothing<rCoord2D>( pith.begin()+firstValidSliceIndex, pith.begin()+lastValidSliceIndex+1, _smoothingRadius );
}


uiCoord2D PithExtractorBoukadida::transHough( const Slice &slice, qreal &lineOnMaxRatio, const rCoord3D &voxelDims, const qreal &adaptativeWidthCoeff ) const
{
	const uint &width = slice.n_cols;
	const uint &height = slice.n_rows;

	const uint semiWidth = qFloor(width/2.);
	const uint semiAdaptativeWidth = qRound(semiWidth*adaptativeWidthCoeff);
	const uint iMin = semiWidth-semiAdaptativeWidth;
	const uint iMax = semiWidth+semiAdaptativeWidth;

	// Calcul des accumulation des droites suivant les orientations
	arma::Mat<int> accuSlice( height, width, arma::fill::zeros );

	lineOnMaxRatio = 0.;

	if ( semiAdaptativeWidth<2 )
		return uiCoord2D(semiWidth,qFloor(height/2.));

	// Calcul des orientations en chaque pixel avec les filtres de Sobel
	arma::Mat<qreal> orientations(0,0);
	arma::Mat<char> hasContour(0,0);
	uint nbContourPoints = contour( slice, orientations, hasContour, voxelDims, adaptativeWidthCoeff );

	uint i, j;
	for ( j=1 ; j<height-1 ; ++j )
	{
		for ( i=iMin ; i<iMax ; ++i )
		{
			if ( hasContour.at(j,i) )
				drawLine( accuSlice, uiCoord2D(i,j), -orientations.at(j,i) );
		}
	}

	// Valeur et coordonnée du maximum de accuSlice
	uiCoord2D pithCoord;
	lineOnMaxRatio = accuSlice.max(pithCoord.y,pithCoord.x);
	lineOnMaxRatio = nbContourPoints ? lineOnMaxRatio/static_cast<qreal>(nbContourPoints) : 0.;

	return pithCoord;
}

uint PithExtractorBoukadida::contour( const Slice &slice, arma::Mat<qreal> & orientations, arma::Mat<char> &hasContour,
									  const rCoord3D &voxelDims, const qreal &adaptativeWidthCoeff ) const
{
	Q_ASSERT_X( adaptativeWidthCoeff>=0. && adaptativeWidthCoeff<=1. , "PithExtractorBoukadida::contour(...)", "adaptativeWidthCoeff must be in [0,1]." );

	const uint &width = slice.n_cols;
	const uint height = slice.n_rows-1;

	orientations.zeros( height+1, width );
	hasContour.zeros( height+1, width );


	const qreal &xDim = voxelDims.x;
	const qreal &yDim = voxelDims.y;
	const qreal voxelRatio = qPow(xDim/yDim,2);

	const uint semiWidth = qFloor(width/2.);
	const uint semiAdaptativeWidth = qRound(semiWidth*adaptativeWidthCoeff);
	const uint iMin = semiWidth-semiAdaptativeWidth+1;
	const uint iMax = semiWidth+semiAdaptativeWidth-1;

	if ( semiAdaptativeWidth < 2 )
		return 0;

	arma::Mat<qreal> sobelNorm(slice.n_rows,iMax-iMin,arma::fill::zeros);

	arma::Col<qreal> sobelNormVec((iMax-iMin)*(height-1));
	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

	uint i, j, nbPositiveNorm;
	qreal sobelX, sobelY, norm;

	nbPositiveNorm = 0;
	for ( i=iMin ; i<iMax ; ++i )
	{
		for ( j=1 ; j<height ; ++j )
		{
			sobelX = slice.at( j-1, i-1 ) - slice.at( j-1, i+1 ) +
					 2* (slice.at( j, i-1 ) - slice.at( j, i+1 )) +
					 slice.at( j+1, i-1 ) - slice.at( j+1, i+1 );
			sobelY = slice.at( j+1, i-1 ) - slice.at( j-1, i-1 ) +
					 2 * (slice.at( j+1, i ) - slice.at( j-1, i )) +
					 slice.at( j+1, i+1 ) - slice.at( j-1, i+1 );
			orientations.at(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX*voxelRatio;
			norm = qSqrt(qPow(sobelX*xDim,2) + qPow(sobelY*yDim,2));
			sobelNorm.at(j,i-iMin) = *sobelNormVecIt++ = norm;
			nbPositiveNorm += !qFuzzyIsNull(norm);
		}
	}

	if ( !nbPositiveNorm  )
		return 0;

	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
	//const qreal &medianVal = sobelNormSort.at( sobelNormSort.n_elem/2 );
	const qreal &medianVal = sobelNormSort.at( sobelNormSort.n_elem - nbPositiveNorm + qRound(nbPositiveNorm/4.) );

	uint nbContourPoints = 0;
	for ( j=1 ; j<height ; ++j )
	{
		for ( i=iMin ; i<iMax ; ++i )
		{
			if ( sobelNorm.at(j,i-iMin) > medianVal )
			{
				nbContourPoints++;
				hasContour.at(j,i) = true;
			}
		}
	}

	return nbContourPoints;
}

void PithExtractorBoukadida::drawLine(arma::Mat<int> &slice, const iCoord2D &origin, const qreal &orientation ) const
{
	const int width = slice.n_cols-1;
	const int height = slice.n_rows-1;
	const int originX = origin.x;
	const int originY = origin.y;
	const qreal orientationInv = 1./orientation;

	qreal x, y;

	if ( orientation >= 1. )
	{
		for ( x = originX , y=originY; x<width && y<height ; x += orientationInv, y += 1. )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1; x>=0. && y>=0. ; x -= orientationInv, y -= 1. )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
	}
	else if ( orientation > 0. )
	{
		for ( x = originX, y=originY ; x<width && y<height ; x += 1., y += orientation )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>=0 && y>=0 ; x -= 1., y -= orientation )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
	}
	else if ( orientation > -1. )
	{
		for ( x = originX, y=originY ; x<width && y>=0 ; x += 1., y += orientation )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>=0 && y<height ; x -= 1., y -= orientation )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
	}
	else
	{
		for ( x = originX , y=originY; x>=0 && y<height ; x += orientationInv, y += 1. )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1.; x<width && y>=0 ; x -= orientationInv, y -= 1. )
		{
			slice.at(qRound(y),qRound(x)) += 1;
		}
	}
}

void PithExtractorBoukadida::interpolation( Pith &pith, const QVector<qreal> &nbLineByMaxRatio, const Interval<uint> &sliceIntervalToInterpolate ) const
{
	const uint &firstSlice = sliceIntervalToInterpolate.min();
	const uint lastSlice = sliceIntervalToInterpolate.max();

	QVector<qreal> nbLineByMaxRatioSorting = nbLineByMaxRatio.mid( firstSlice, lastSlice-firstSlice+1 );
	qSort(nbLineByMaxRatioSorting);

	const qreal &quartile1 = nbLineByMaxRatioSorting[ 0.25 * nbLineByMaxRatioSorting.size() ];
	const qreal &quartile3 = nbLineByMaxRatioSorting[ 0.75 * nbLineByMaxRatioSorting.size() ];
	const qreal interpolationThreshold = quartile1 - 0.5 * ( quartile3 - quartile1 );

	uint startSliceIndex, newK, startSliceIndexMinusOne;
	rCoord2D interpolationStep, currentInterpolatePithCoord;
	for ( uint k=firstSlice ; k<lastSlice ; ++k )
	{
		if ( nbLineByMaxRatio[k] < interpolationThreshold )
		{
			startSliceIndex = k++;
			startSliceIndexMinusOne = startSliceIndex?startSliceIndex-1:0;

			while ( k<lastSlice && nbLineByMaxRatio[k] < interpolationThreshold ) ++k;
			if ( k<lastSlice ) --k;

			std::cout << "Interpolation [" << startSliceIndex << ", " << k << "]" << std::endl;

			interpolationStep = startSliceIndex && k<lastSlice ? (pith[k+1] - pith[startSliceIndexMinusOne]) / static_cast<qreal>( k+1-startSliceIndexMinusOne )
															   : rCoord2D(0,0);

			currentInterpolatePithCoord = interpolationStep +
										  (startSliceIndex ? pith[startSliceIndexMinusOne]
														   : pith[k+1]);

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
	const uint &width = billonToFill.n_cols;
	const uint &height = billonToFill.n_rows;
	const uint &nbSlices = billonToFill.n_slices;
	const int &minIntensity = intensityInterval.min();
	const int &maxIntensity = intensityInterval.max();

	const uint semiWidth = qFloor(width/2.);

	backgroundProportions.resize(nbSlices);

	Slice::col_iterator begin, end;
	uint k, semiAdaptativeWidth, iMin, iMax;
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
			if ( semiAdaptativeWidth<1 ) continue;
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

Interval<uint> PithExtractorBoukadida::detectValidSliceInterval( const QVector<qreal> &backgroundProportions ) const
{
	const uint &nbSlices = backgroundProportions.size();
	const qreal backgroundPercentage = (100.-_minWoodPercentage)/100.;
	uint sliceIndex;

	Interval<uint> validSliceInterval(0,nbSlices-1);

	sliceIndex = 0;
	while ( sliceIndex<nbSlices && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex++;
	validSliceInterval.setMin(qMin(sliceIndex,nbSlices-1));

	const uint &minValid = validSliceInterval.min();
	sliceIndex = nbSlices-1;
	while ( sliceIndex>minValid && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex--;
	validSliceInterval.setMax(sliceIndex);

	return validSliceInterval;
}
