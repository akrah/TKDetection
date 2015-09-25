#include <QTextCodec>
#include <QSettings>

#include "def/def_billon.h"
#include "inc/billon.h"

#include "inc/define.h"

#include "inc/dicomreader.h"
#include "inc/pithextractorboukadida.h"
#include "inc/billonalgorithms.h"
#include "inc/detection/knotbywhorldetector.h"

#include "inc/segmentation/tangentialgenerator.h"
#include "inc/segmentation/pithprofile.h"
#include "inc/segmentation/ellipseradiihistogram.h"

struct Params_Global {
	int minIntensity = MINIMUM_INTENSITY;
	int maxIntensity = MAXIMUM_INTENSITY;
	int minZMotion = MINIMUM_Z_MOTION;
};

struct Params_PithDetectionBillon {
	int minIntensity = MINIMUM_INTENSITY;
	int maxIntensity = MAXIMUM_INTENSITY;
	int minZMotion = MINIMUM_Z_MOTION;
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON ;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON ;
	int pithShift = PITH_LAG_BILLON ;
	int smoothingRadius = PITH_SMOOTHING_BILLON ;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_BILLON ;
	bool useAscendingOrder = ASCENDING_ORDER_BILLON ;
	bool useExtrapolation = TKD::LINEAR ;
};


struct Params_RestrictedArea {
	int nbDirections = RESTRICTED_AREA_DEFAULT_RESOLUTION;
	int barkValue = MINIMUM_INTENSITY;
	int minRadius = 5;
};

struct Params_SliceHistogram {
	int smoothingRadius = HISTOGRAM_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS;
};

struct Params_SectorHistogram {
	int smoothingRadius = HISTOGRAM_ANGULAR_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS;
};

struct Params_PithDetectionKnot {
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_KNOT ;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_KNOT ;
	int pithShift = PITH_LAG_KNOT ;
	int smoothingRadius = PITH_SMOOTHING_KNOT ;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_KNOT ;
	bool useAscendingOrder = ASCENDING_ORDER_KNOT ;
	bool useExtrapolation = TKD::SLOPE_DIRECTION ;
	int percentageOfFirstValidSlicesToExtrapolate = FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT ;
	int percentageOfLastValidSlicesToExtrapolate = LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT ;
};

struct Params_TangentialGenerator {
	bool useTrilinearInterpolation = true;
};

struct Params_PithProfile {
	int smoothingRadius = 2;
};


struct Params_EllipticalHistograms {
	int lowessBandWidth = 33;
	int lowessIqrCoefficient  = 1;
	int lowessPercentageOfFirstValidSlicesToExtrapolate = 5;
	int lowessPercentageOfLastValidSlicesToExtrapolate = 0;
	int smoothingRadius = 0;
};

void computeBillonPith( Billon& billon, qreal &treeRadius );
void detectKnotsByWhorls( const Billon &billon , KnotAreaDetector* knotByWhorlDetector, const qreal &treeRadius );
void segmentKnots( const Billon &billon, const KnotAreaDetector &detector );
Billon* segmentKnotArea( const Billon &billon, const KnotArea &supportingArea,
						 const PieChart& pieChart, TangentialGenerator &tangentialGenerator,
						 PithExtractorBoukadida& knotPithExtractor );

int main( int argc, char *argv[] )
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	/********************************************************************/
	qDebug() << "1) Lecture es paramètres...";
	QSettings settings("tkd_shell.ini",QSettings::IniFormat);

	Params_Global::minIntensity = MINIMUM_INTENSITY;
	Params_Global::maxIntensity = MAXIMUM_INTENSITY;
	Params_Global::minZMotion = MINIMUM_Z_MOTION;

	Params_PithDetectionBillon::subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON ;
	Params_PithDetectionBillon::subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON ;
	Params_PithDetectionBillon::pithShift = PITH_LAG_BILLON ;
	Params_PithDetectionBillon::smoothingRadius = PITH_SMOOTHING_BILLON ;
	Params_PithDetectionBillon::minimumWoodPercentage = MIN_WOOD_PERCENTAGE_BILLON ;
	Params_PithDetectionBillon::useAscendingOrder = ASCENDING_ORDER_BILLON ;
	Params_PithDetectionBillon::useExtrapolation = TKD::LINEAR ;

	Params_RestrictedArea::nbDirections = RESTRICTED_AREA_DEFAULT_RESOLUTION;
	Params_RestrictedArea::barkValue = MINIMUM_INTENSITY;
	Params_RestrictedArea::minRadius = 5;

	Params_SliceHistogram::smoothingRadius = HISTOGRAM_SMOOTHING_RADIUS;
	Params_SliceHistogram::minimumHeightOfMaximums = HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	Params_SliceHistogram::derivativeSearchPercentage = HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE;
	Params_SliceHistogram::minimumWidthOfIntervals = HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS;

	Params_SectorHistogram::smoothingRadius = HISTOGRAM_ANGULAR_SMOOTHING_RADIUS;
	Params_SectorHistogram::minimumHeightOfMaximums = HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	Params_SectorHistogram::derivativeSearchPercentage = HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE;
	Params_SectorHistogram::minimumWidthOfIntervals = HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS;

	Params_PithDetectionKnot::subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_KNOT ;
	Params_PithDetectionKnot::subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_KNOT ;
	Params_PithDetectionKnot::pithShift = PITH_LAG_KNOT ;
	Params_PithDetectionKnot::smoothingRadius = PITH_SMOOTHING_KNOT ;
	Params_PithDetectionKnot::minimumWoodPercentage = MIN_WOOD_PERCENTAGE_KNOT ;
	Params_PithDetectionKnot::useAscendingOrder = ASCENDING_ORDER_KNOT ;
	Params_PithDetectionKnot::useExtrapolation = TKD::SLOPE_DIRECTION ;
	Params_PithDetectionKnot::percentageOfFirstValidSlicesToExtrapolate = FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT ;
	Params_PithDetectionKnot::percentageOfLastValidSlicesToExtrapolate = LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT ;

	Params_TangentialGenerator::useTrilinearInterpolation = true;

	Params_PithProfile::smoothingRadius = 2;

	Params_EllipticalHistograms::lowessBandWidth = 33;
	Params_EllipticalHistograms::lowessIqrCoefficient  = 1;
	Params_EllipticalHistograms::lowessPercentageOfFirstValidSlicesToExtrapolate = 5;
	Params_EllipticalHistograms::lowessPercentageOfLastValidSlicesToExtrapolate = 0;
	Params_EllipticalHistograms::smoothingRadius = 0;

	return 0;

	Billon *_billon = 0;
	KnotAreaDetector* _knotByWhorlDetector = new KnotByWhorlDetector;

	/********************************************************************/
	qDebug() << "1) Lecture du dicom...";

	QString folderName("/home/akrah/LaBRI/Images/PoumonsIRM/TEST_GD2/1/8/");
	if ( folderName.isEmpty() )
	{
		qDebug() << "ERREUR : le répertoire spécifié n'existe pas.";
		delete _knotByWhorlDetector;
		return 1;
	}

	_billon = DicomReader::read( folderName );
	if ( !_billon )
	{
		qDebug() << "ERREUR : le chargement de l'image a échoué.";
		delete _knotByWhorlDetector;
		return 2;
	}

	/********************************************************************/
	qDebug() << "2) Calcul de la moelle du tronc...";

	qreal _treeRadius;

	computeBillonPith( *_billon, _treeRadius );
	if ( !_billon->hasPith() )
	{
		qDebug() << "ERREUR : la moelle du tronc n'a pas pu être calculée.";
		delete _billon;
		delete _knotByWhorlDetector;
		return 3;
	}

	/********************************************************************/
	qDebug() << "2) Détection des zones de nœuds...";

	detectKnotsByWhorls( *_billon, _knotByWhorlDetector, _treeRadius );

	/********************************************************************/
	qDebug() << "2) Segmentation des zones de nœuds...";

	segmentKnots(*_billon,*_knotByWhorlDetector);

	/********************************************************************/
	qDebug() << "";
	qDebug() << "Finit !";

	delete _knotByWhorlDetector;
	if ( _billon )
	{
		delete _billon;
	}

	return 0;
}

void computeBillonPith( Billon& billon, qreal &treeRadius )
{
	 PithExtractorBoukadida _billonPithExtractor;

	_billonPithExtractor.setSubWindowWidth( NEIGHBORHOOD_WINDOW_WIDTH_BILLON );
	_billonPithExtractor.setSubWindowHeight( NEIGHBORHOOD_WINDOW_HEIGHT_BILLON );
	_billonPithExtractor.setPithShift( PITH_LAG_BILLON );
	_billonPithExtractor.setSmoothingRadius( PITH_SMOOTHING_BILLON );
	_billonPithExtractor.setMinWoodPercentage( MIN_WOOD_PERCENTAGE_BILLON );
	_billonPithExtractor.setIntensityInterval( Interval<int>( MINIMUM_INTENSITY, MAXIMUM_INTENSITY ) );
	_billonPithExtractor.setAscendingOrder( ASCENDING_ORDER_BILLON );
	_billonPithExtractor.setExtrapolation( TKD::LINEAR );
	_billonPithExtractor.setFirstValidSlicesToExtrapolate( FIRST_VALID_SLICES_TO_EXTRAPOLATE_BILLON );
	_billonPithExtractor.setLastValidSlicesToExtrapolate( LAST_VALID_SLICES_TO_EXTRAPOLATE_BILLON );

	_billonPithExtractor.process( billon );

	treeRadius = BillonAlgorithms::restrictedAreaMeansRadius(billon,
															 RESTRICTED_AREA_DEFAULT_RESOLUTION,
															 RESTRICTED_AREA_DEFAULT_THRESHOLD,
															 5*billon.n_cols/100.);
}

void detectKnotsByWhorls( const Billon &billon, KnotAreaDetector* knotByWhorlDetector, const qreal &treeRadius )
{
	knotByWhorlDetector->setIntensityInterval( Interval<int>(MINIMUM_INTENSITY,MAXIMUM_INTENSITY) );
	knotByWhorlDetector->setZMotionMin( MINIMUM_Z_MOTION );
	knotByWhorlDetector->setTreeRadius( treeRadius );
	static_cast<KnotByWhorlDetector*>(knotByWhorlDetector)->setSliceHistogramParameters(
				HISTOGRAM_SMOOTHING_RADIUS,
				HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM,
				HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE,
				HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS);

	const qreal coeffDegToSize = knotByWhorlDetector->pieChart().nbSectors()/360.;
	static_cast<KnotByWhorlDetector*>(knotByWhorlDetector)->setSectorHistogramsParameters(
				HISTOGRAM_ANGULAR_SMOOTHING_RADIUS*coeffDegToSize,
				HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM,
				HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE,
				HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS*coeffDegToSize);

	knotByWhorlDetector->execute( billon );
}

void segmentKnots( const Billon &billon, const KnotAreaDetector &detector )
{
	const bool enableTrilinearInterpolation = true;

	TangentialGenerator tangentialGenerator(-900, true);
	tangentialGenerator.enableTrilinearInterpolation( enableTrilinearInterpolation );
	tangentialGenerator.setMinIntensity( MINIMUM_INTENSITY );

	PithExtractorBoukadida knotPithExtractor;
	knotPithExtractor.setSubWindowWidth( NEIGHBORHOOD_WINDOW_WIDTH_KNOT );
	knotPithExtractor.setSubWindowHeight( NEIGHBORHOOD_WINDOW_HEIGHT_KNOT );
	knotPithExtractor.setPithShift( PITH_LAG_KNOT );
	knotPithExtractor.setSmoothingRadius( PITH_SMOOTHING_KNOT );
	knotPithExtractor.setMinWoodPercentage( MIN_WOOD_PERCENTAGE_KNOT );
	knotPithExtractor.setIntensityInterval( Interval<int>( MINIMUM_INTENSITY, MAXIMUM_INTENSITY ) );
	knotPithExtractor.setAscendingOrder( ASCENDING_ORDER_KNOT );
	knotPithExtractor.setExtrapolation( TKD::SLOPE_DIRECTION );
	knotPithExtractor.setFirstValidSlicesToExtrapolate( FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT );
	knotPithExtractor.setLastValidSlicesToExtrapolate( LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT );

	Billon* tangentialBillon = 0;

	const int nbKnotAreas = detector.knotAreas().size();
	int i = 0;

	QVectorIterator<KnotArea> supportingAreasIter(detector.knotAreas());
	while ( supportingAreasIter.hasNext() )
	{
		qDebug() << "    Noeud " << i++ << " / " << nbKnotAreas << " ...";
		const KnotArea &supportingArea = supportingAreasIter.next();
		tangentialBillon = segmentKnotArea( billon, supportingArea, detector.pieChart(), tangentialGenerator, knotPithExtractor );
		if ( tangentialBillon )
		{
			qDebug() << "        -> OK !";
			delete tangentialBillon;
		}
		else
		{
			qDebug() << "        -> NONNNNNNNNNNNNNNNNNNNNNNNNNNN !!!";
		}
		tangentialBillon = 0;
	}

	if ( tangentialBillon )
	{
		delete tangentialBillon;
	}
}

Billon* segmentKnotArea( const Billon &billon, const KnotArea &supportingArea,
						 const PieChart& pieChart, TangentialGenerator &tangentialGenerator,
						 PithExtractorBoukadida& knotPithExtractor )
{
	const int pithProfileSmoothing = 2;

	const int lowessBandWidth = 33;
	const int lowessIqrCoefficient  = 1;
	const int lowessPercentageOfFirstValidSlicesToExtrapolate = 5;
	const int lowessPercentageOfLastValidSlicesToExtrapolate = 0;
	const int ellipticalAccumulationSmoothingRadius = 0;

	Billon* tangentialBillon = 0;
	PithProfile knotPithProfile;
	EllipseRadiiHistogram knotEllipseRadiiHistogram;

	const Interval<uint> &sliceInterval = supportingArea.sliceInterval();
	const Interval<uint> &sectorInterval = supportingArea.sectorInterval();

	tangentialGenerator.updateIntervals( billon, sliceInterval, sectorInterval, pieChart );

	tangentialBillon = tangentialGenerator.execute( billon, pieChart );

	if ( tangentialBillon )
	{
		knotPithExtractor.process(*tangentialBillon,true);
	}

	if ( tangentialBillon->hasPith() )
	{
		knotPithProfile.construct( tangentialBillon->pith(), pithProfileSmoothing );
	}

	if ( tangentialBillon->hasPith() && knotPithProfile.size() )
	{
		knotEllipseRadiiHistogram.construct( *tangentialBillon,
											 knotPithProfile,
											 lowessBandWidth,
											 ellipticalAccumulationSmoothingRadius,
											 lowessIqrCoefficient,
											 lowessPercentageOfFirstValidSlicesToExtrapolate,
											 lowessPercentageOfLastValidSlicesToExtrapolate );
	}

	return tangentialBillon;
}
