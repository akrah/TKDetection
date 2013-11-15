#include "inc/pithextractorboukadida.h"

#include "inc/billon.h"
#include "inc/coordinate.h"

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

void PithExtractorBoukadida::process( Billon &billon ) const
{
	const int &width = billon.n_cols-1;
	const int &height = billon.n_rows-1;
	const uint &depth = billon.n_slices;
	const int semiSubWindowWidth = _subWindowWidth/2;
	const int semiSubWindowHeight = _subWindowHeight/2;
	const uint kIncrement = (_ascendingOrder?1:-1);
	Pith &pith = billon._pith;

	uint k;

	pith.clear();
	pith.resize(depth);

	QVector<qreal> nbLineByMaxRatio( depth, 0. );
	QVector<qreal> backgroundProportions;

	std::cout << "Step 1] Copie du billon" << std::endl;
	Billon billonFillBackground(billon);

	std::cout << "Step 2] Fill billon background and compute background proportions" << std::endl;
	fillBillonBackground( billonFillBackground, backgroundProportions, _intensityInterval );

	std::cout << "Step 3] Detect interval of valid slices" << std::endl;
	const Interval<uint> validSliceInterval = detectValidSliceInterval( backgroundProportions );
	const uint &firstValideSliceIndex = validSliceInterval.min();
	const uint &lastValideSliceIndex = validSliceInterval.max();
	const uint firstSliceOrdered = _ascendingOrder?firstValideSliceIndex:lastValideSliceIndex;

	// Calcul de la moelle sur la première coupe valide
	std::cout << "Step 4] Hough transform on first valid slice" << std::endl;
	pith[firstSliceOrdered] = transHough( billonFillBackground.slice(firstSliceOrdered), nbLineByMaxRatio[firstSliceOrdered] );

	// Calcul de la moelle sur les coupes suivantes
	std::cout << "Step 5] Hough transform on next valid slices" << std::endl;
	uiCoord2D currentPithCoord;
	uiCoord2D subWindowStart, subWindowEnd;
	for ( k=firstSliceOrdered+kIncrement ; k<=lastValideSliceIndex && k>=firstValideSliceIndex ; k += kIncrement )
	{
		std::cout << k << "  ";
		const Slice &currentSlice = billonFillBackground.slice(k);
		const iCoord2D &previousPith = pith[k-kIncrement];

		subWindowStart.x = qMax(qMin(previousPith.x-semiSubWindowWidth,width),0);
		subWindowEnd.x = qMax(qMin(previousPith.x+semiSubWindowWidth,width),0);
		subWindowStart.y = qMax(qMin(previousPith.y-semiSubWindowHeight,height),0);
		subWindowEnd.y = qMax(qMin(previousPith.y+semiSubWindowHeight,height),0);
		currentPithCoord = transHough( currentSlice.submat( subWindowStart.y, subWindowStart.x, subWindowEnd.y, subWindowEnd.x ), nbLineByMaxRatio[k] )
						   + subWindowStart;

		if ( currentPithCoord.euclideanDistance(previousPith) > _pithShift )
		{
			std::cout << "...  " << std::endl;
			currentPithCoord = transHough( currentSlice, nbLineByMaxRatio[k] );
		}

		pith[k] = currentPithCoord;
	}
	std::cout << std::endl;

	// Extrapolation des coupes invalides
	std::cout << "Step 6] Extrapolation of unvali slices" << std::endl;
	const rCoord2D &firstValidCoord = pith[firstValideSliceIndex];
	for ( k=0 ; k<firstValideSliceIndex ; ++k )
	{
		pith[k] = firstValidCoord;
	}
	const rCoord2D &lastValidCoord = pith[lastValideSliceIndex];
	for ( k=lastValideSliceIndex+1 ; k<depth ; ++k )
	{
		pith[k] = lastValidCoord;
	}

	// Interpolation des coupes valides
	std::cout << "Step 7] Interpolation of valid slices" << std::endl;
	interpolation( pith, nbLineByMaxRatio, validSliceInterval );

	// Lissage
	std::cout << "Step 8] Smoothing of valid slices" << std::endl;
	smoothing( pith, _smoothingRadius, validSliceInterval );
}


uiCoord2D PithExtractorBoukadida::transHough( const Slice &slice, qreal &lineOnMaxRatio ) const
{
	const uint &width = slice.n_cols;
	const uint &height = slice.n_rows;

	// Calcul des orientations en chaque pixel avec les filtres de Sobel
	arma::Mat<qreal> orientations( height, width );
	arma::Mat<char> hasContour( height, width );
	uint nbContourPoints = contour( slice, orientations, hasContour );

	// Calcul des accumulation des droites suivant les orientations
	arma::Mat<int> accuSlice( height, width );
	accuSlice.fill(0);

	uint i, j;
	for ( j=1 ; j<height-1 ; ++j )
	{
		for ( i=1 ; i<width-1 ; ++i )
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

uint PithExtractorBoukadida::contour( const Slice &slice, arma::Mat<qreal> & orientations, arma::Mat<char> &hasContour ) const
{
	const uint &width = slice.n_cols-1;
	const uint &height = slice.n_rows-1;

	arma::Mat<qreal> sobelNorm(slice.n_rows,slice.n_cols);
	sobelNorm.fill(0.);

	arma::Col<qreal> sobelNormVec((width-1)*(height-1));
	arma::Col<qreal>::iterator sobelNormVecIt = sobelNormVec.begin();

	uint i, j;
	qreal sobelX, sobelY;

	orientations.fill(0);
	for ( j=1 ; j<height ; ++j )
	{
		for ( i=1 ; i<width ; ++i )
		{
			sobelX = slice.at( j-1, i-1 ) - slice.at( j-1, i+1 ) +
					 2* (slice.at( j, i-1 ) - slice.at( j, i+1 )) +
					 slice.at( j+1, i-1 ) - slice.at( j+1, i+1 );
			sobelY = slice.at( j+1, i-1 ) - slice.at( j-1, i-1 ) +
					 2 * (slice.at( j+1, i ) - slice.at( j-1, i )) +
					 slice.at( j+1, i+1 ) - slice.at( j-1, i+1 );
			orientations.at(j,i) = qFuzzyIsNull(sobelX) ? 9999999999./1. : sobelY/sobelX;
			sobelNorm.at(j,i) = qSqrt( qPow(sobelX,2) + qPow(sobelY,2) )/4.;
			*(sobelNormVecIt++) = sobelNorm.at(j,i);
		}
	}

	const arma::Col<qreal> sobelNormSort = arma::sort( sobelNormVec );
	const qreal &medianVal = sobelNormSort.at( sobelNormSort.n_elem/2 );

	uint nbContourPoints = 0;
	hasContour.fill(false);
	for ( j=1 ; j<height ; ++j )
	{
		for ( i=1 ; i<width ; ++i )
		{
			if ( sobelNorm.at(j,i) > medianVal )
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
	const int &width = slice.n_cols;
	const int &height = slice.n_rows;
	const int originX = origin.x;
	const int originY = origin.y;
	const qreal orientationInv = 1./orientation;

	qreal x, y;

	if ( orientation >= 1. )
	{
		for ( x = originX , y=originY; x<width && y<height ; x += orientationInv, y += 1. )
		{
			slice.at(y,x) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1; x>=0. && y>=0. ; x -= orientationInv, y -= 1. )
		{
			slice.at(y,x) += 1;
		}
	}
	else if ( orientation > 0. )
	{
		for ( x = originX, y=originY ; x<width && y<height ; x += 1., y += orientation )
		{
			slice.at(y,x) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>=0 && y>=0 ; x -= 1., y -= orientation )
		{
			slice.at(y,x) += 1;
		}
	}
	else if ( orientation > -1. )
	{
		for ( x = originX, y=originY ; x<width && y>=0 ; x += 1., y += orientation )
		{
			slice.at(y,x) += 1;
		}
		for ( x = originX-1., y=originY-orientation ; x>=0 && y<height ; x -= 1., y -= orientation )
		{
			slice.at(y,x) += 1;
		}
	}
	else
	{
		for ( x = originX , y=originY; x>=0 && y<height ; x += orientationInv, y += 1. )
		{
			slice.at(y,x) += 1;
		}
		for ( x = originX-orientationInv , y=originY-1.; x<width && y>=0 ; x -= orientationInv, y -= 1. )
		{
			slice.at(y,x) += 1;
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
	for ( uint k=firstSlice ; k<lastSlice-2 ; ++k )
	{
		if ( nbLineByMaxRatio[k] < interpolationThreshold )
		{
			startSliceIndex = k++;
			startSliceIndexMinusOne = startSliceIndex?startSliceIndex-1:0;

			while ( k<lastSlice && nbLineByMaxRatio[k] < interpolationThreshold ) ++k;

			interpolationStep = (pith[k] - pith[startSliceIndexMinusOne]) / static_cast<qreal>( k-startSliceIndexMinusOne+1 );

			currentInterpolatePithCoord = interpolationStep + pith[startSliceIndexMinusOne];
			for ( newK = startSliceIndexMinusOne+1 ; newK < k ; ++newK, currentInterpolatePithCoord += interpolationStep )
			{
				pith[newK] = currentInterpolatePithCoord;
			}
		}
	}
}

void PithExtractorBoukadida::smoothing( Pith &pith, const uint &smoothingRadius, const Interval<uint> &sliceIntervalToSmooth ) const
{
	if ( !smoothingRadius || sliceIntervalToSmooth.size() < 1 || pith.size() < static_cast<int>(2*smoothingRadius+1) )
		return;

	const uint &firstSliceToSmooth = sliceIntervalToSmooth.min();
	const uint &lastSliceToSmooth = sliceIntervalToSmooth.max();
	const uint nbSlicesToSmooth = sliceIntervalToSmooth.size()+1;
	const qreal smoothingMaskSize = 2.*smoothingRadius+1.;

	uint i;

	Pith pithCopy;
	pithCopy.reserve( nbSlicesToSmooth + 2*smoothingRadius );
	for ( i=0 ; i<smoothingRadius ; ++i ) pithCopy << pith[firstSliceToSmooth];
	pithCopy << pith.mid(firstSliceToSmooth, nbSlicesToSmooth);
	for ( i=0 ; i<smoothingRadius ; ++i ) pithCopy << pith[lastSliceToSmooth];

	Pith::ConstIterator copyIterBegin = pithCopy.constBegin();
	Pith::ConstIterator copyIterEnd = pithCopy.constBegin() + static_cast<int>(smoothingMaskSize);

	Pith::Iterator pithIter = pith.begin()+firstSliceToSmooth;
	const Pith::ConstIterator pithEnd = pith.begin()+lastSliceToSmooth+1;

	rCoord2D currentValue = std::accumulate( copyIterBegin, copyIterEnd, rCoord2D(0.,0.) );
	*pithIter++ = currentValue/smoothingMaskSize;
	while ( pithIter != pithEnd )
	{
		currentValue += (*copyIterEnd++ - *copyIterBegin++);
		*pithIter++ = currentValue/smoothingMaskSize;
	}
}

void PithExtractorBoukadida::fillBillonBackground( Billon &billonToFill, QVector<qreal> &backgroundProportions, const Interval<int> &intensityInterval ) const
{
	const uint &nbSlices = billonToFill.n_slices;
	const int &minIntensity = intensityInterval.min()+1;
	const int &maxIntensity = intensityInterval.max()-1;
	const qreal nbVoxelsBySlice = billonToFill.n_cols * billonToFill.n_rows;

	backgroundProportions.resize(nbSlices);

	Slice::iterator startSlice, endSlice;
	uint k;
	qreal currentProp;
	__billon_type__ val;
	for ( k=0 ; k<nbSlices ; ++k )
	{
		startSlice = billonToFill.slice(k).begin();
		endSlice = billonToFill.slice(k).end();
		currentProp = 0.;
		while ( startSlice != endSlice )
		{
			val = *startSlice;
			*startSlice = qMax( qMin( val, maxIntensity ), minIntensity );
			currentProp += (val != *startSlice++);
		}
		backgroundProportions[k] = currentProp / nbVoxelsBySlice;
	}
}

Interval<uint> PithExtractorBoukadida::detectValidSliceInterval( const QVector<qreal> &backgroundProportions ) const
{
	const uint &nbSlices = backgroundProportions.size();
	const qreal backgroundPercentage = 1.-_minWoodPercentage/100.;
	uint sliceIndex;

	Interval<uint> validSliceInterval(0,nbSlices-1);

	sliceIndex = 0;
	while ( sliceIndex<nbSlices && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex++;
	validSliceInterval.setMin(qMin(sliceIndex,nbSlices-1));

	sliceIndex = nbSlices-1;
	while ( sliceIndex>0 && backgroundProportions[sliceIndex] > backgroundPercentage ) sliceIndex--;
	validSliceInterval.setMax(qMax(sliceIndex,validSliceInterval.min()));

	return validSliceInterval;
}
