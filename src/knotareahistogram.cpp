//#include "inc/knotareahistogram.h"

//KnotAreaHistogram::KnotAreaHistogram() : Histogram<qreal>()
//{
//}

//KnotAreaHistogram::~KnotAreaHistogram()
//{
//}


///**********************************
// * Public setters
// **********************************/

//void KnotAreaHistogram::construct( const Billon &billon, const Pith &pith, const Interval<int> &intensity, const Interval<int> &motionInterval, const int &borderPercentageToCut,
//				const int & radiusAroundPith )
//{
//	const int nbComponents = billon->maxValue();
//	const int depth = billon->n_slices;

//	QVector<QwtIntervalSample> histData(depth,QwtIntervalSample(0,0,0));
//	Billon *biggestComponents;
//	iCoord2D nearestPoint;

//	int minIndex;
//	qreal minVal;

//	minIndex = 0;
//	minVal = width;
//	for ( i=0 ; i<depth ; ++i )
//	{
//		biggestComponents = ConnexComponentExtractor::extractConnexComponents( billon->slice(i), qPow(_ui->_spinMinimalSizeOf2DConnexComponents->value(),2), 0 );
//		nearestPoint = biggestComponents->findNearestPointOfThePith( (*_pith)[firstSlice+i], 0, 0 );
//		//nearestPoint = billon->findNearestPointOfThePith( (*_pith)[firstSlice+i], i, 0 );
//		histData[i].value = nearestPoint.euclideanDistance((*_pith)[firstSlice+i]);
//		histData[i].interval.setInterval(i,i+1);
//		if ( minVal > histData[i].value )
//		{
//			minVal = histData[i].value;
//			minIndex = i;
//		}
//		delete biggestComponents;
//		biggestComponents = 0;
//	}

//	_histogramDistancePithToNearestPoint.setSamples(histData);
//	_ui->_plotDistancePithToNearestPoint->replot();

//	int upperIndex, lowerIndex;

//	upperIndex = qMin(depth-5,minIndex);
//	while ( upperIndex < depth-4 && (histData[upperIndex+5].value - histData[upperIndex].value > 5.) ) upperIndex++;
//	lowerIndex = qMax(5,minIndex);
//	while ( lowerIndex > 3 && histData[lowerIndex-5].value - histData[lowerIndex].value > 5. ) lowerIndex--;

//	_knotIntervalInDistancePithToNearestPointHistogram.setBounds(lowerIndex+3,upperIndex-3);

//	std::cout << "Bornes du noeud : [ " << lowerIndex+1+firstSlice << ", " << upperIndex-1+firstSlice << " ]" << std::endl;
//}
