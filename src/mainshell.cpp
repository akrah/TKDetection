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

/*********************************************************
 * Déclaration des strctures pour les paramètres des algos
 * *******************************************************/
struct Params_Global {
	int minIntensity = MINIMUM_INTENSITY;
	int maxIntensity = MAXIMUM_INTENSITY;
	int minZMotion = MINIMUM_Z_MOTION;
} p_global;

struct Params_PithDetectionBillon {
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_BILLON;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_BILLON;
	int pithShift = PITH_LAG_BILLON;
	int smoothingRadius = PITH_SMOOTHING_BILLON;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_BILLON;
	bool useAscendingOrder = ASCENDING_ORDER_BILLON;
	int extrapolationType = TKD::LINEAR;
	int percentageOfFirstValidSlicesToExtrapolate = 0;
	int percentageOfLastValidSlicesToExtrapolate = 0;
} p_pithDetectionBillon;

struct Params_RestrictedArea {
	int nbDirections = RESTRICTED_AREA_DEFAULT_RESOLUTION;
	int barkValue = MINIMUM_INTENSITY;
	int minRadius = 5;
} p_restrictedArea;

struct Params_SliceHistogram {
	int smoothingRadius = HISTOGRAM_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS;
} p_sliceHistogram;

struct Params_SectorHistogram {
	int smoothingRadius = HISTOGRAM_ANGULAR_SMOOTHING_RADIUS;
	int minimumHeightOfMaximums = HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM;
	int derivativeSearchPercentage = HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE;
	int minimumWidthOfIntervals = HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS;
} p_sectorHistogram;

struct Params_PithDetectionKnot {
	int subWindowWidth = NEIGHBORHOOD_WINDOW_WIDTH_KNOT;
	int subWindowHeight = NEIGHBORHOOD_WINDOW_HEIGHT_KNOT;
	int pithShift = PITH_LAG_KNOT;
	int smoothingRadius = PITH_SMOOTHING_KNOT;
	int minimumWoodPercentage = MIN_WOOD_PERCENTAGE_KNOT;
	bool useAscendingOrder = ASCENDING_ORDER_KNOT;
	int extrapolationType = TKD::SLOPE_DIRECTION;
	int percentageOfFirstValidSlicesToExtrapolate = FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT;
	int percentageOfLastValidSlicesToExtrapolate = LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT;
} p_pithDetectionKnot;

struct Params_TangentialGenerator {
	bool useTrilinearInterpolation = true;
} p_tangentialGenerator;

struct Params_PithProfile {
	int smoothingRadius = 2;
} p_pithProfile;

struct Params_EllipticalHistograms {
	int lowessBandWidth = 33;
	int lowessIqrCoefficient  = 1;
	int lowessPercentageOfFirstValidSlicesToExtrapolate = 5;
	int lowessPercentageOfLastValidSlicesToExtrapolate = 0;
	int smoothingRadius = 0;
} p_ellipticalHistograms;


/**************************************************
 * Déclaration des fonctions appellées dans le main
 * ************************************************/
void computeBillonPith( Billon& billon, qreal &treeRadius );
void detectKnotsByWhorls( const Billon &billon , KnotAreaDetector* knotByWhorlDetector, const qreal &treeRadius );
void segmentKnots( const Billon &billon, const KnotAreaDetector &detector );
Billon* segmentKnotArea( const Billon &billon, const KnotArea &supportingArea,
						 const PieChart& pieChart, TangentialGenerator &tangentialGenerator,
						 PithExtractorBoukadida& knotPithExtractor );


/*********************
 * Fonciton principale
 * *******************/
int main( int argc, char *argv[] )
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	/********************************************************************/
	qDebug() << "1) Lecture es paramètres...";
	QSettings settings("tkd_shell.ini",QSettings::IniFormat);

	p_global.minIntensity = settings.value("General/minIntensity",MINIMUM_INTENSITY).toInt();
	p_global.maxIntensity = settings.value("General/",MAXIMUM_INTENSITY).toInt();
	p_global.minZMotion = settings.value("General/",MINIMUM_Z_MOTION).toInt();

	p_pithDetectionBillon.subWindowWidth = settings.value("PithDetectionBillon/subWindowWidth",NEIGHBORHOOD_WINDOW_WIDTH_BILLON).toInt();
	p_pithDetectionBillon.subWindowHeight = settings.value("PithDetectionBillon/subWindowHeight",NEIGHBORHOOD_WINDOW_HEIGHT_BILLON).toInt();
	p_pithDetectionBillon.pithShift = settings.value("PithDetectionBillon/pithShift",PITH_LAG_BILLON).toInt();
	p_pithDetectionBillon.smoothingRadius = settings.value("PithDetectionBillon/smoothingRadius",PITH_SMOOTHING_BILLON).toInt();
	p_pithDetectionBillon.minimumWoodPercentage = settings.value("PithDetectionBillon/minimumWoodPercentage",MIN_WOOD_PERCENTAGE_BILLON).toInt();
	p_pithDetectionBillon.useAscendingOrder = settings.value("PithDetectionBillon/useAscendingOrder",ASCENDING_ORDER_BILLON).toBool();
	p_pithDetectionBillon.extrapolationType = settings.value("PithDetectionBillon/extrapolationType",TKD::LINEAR).toInt();
	p_pithDetectionBillon.percentageOfFirstValidSlicesToExtrapolate = settings.value("PithDetectionBillon/percentageOfFirstValidSlicesToExtrapolate",0).toInt();
	p_pithDetectionBillon.percentageOfLastValidSlicesToExtrapolate = settings.value("PithDetectionBillon/percentageOfLastValidSlicesToExtrapolate",0).toInt();

	p_restrictedArea.nbDirections = settings.value("RestrictedArea/nbDirections",RESTRICTED_AREA_DEFAULT_RESOLUTION).toInt();
	p_restrictedArea.barkValue = settings.value("RestrictedArea/barkValue",MINIMUM_INTENSITY).toInt();
	p_restrictedArea.minRadius = settings.value("RestrictedArea/minRadius",5).toInt();

	p_sliceHistogram.smoothingRadius = settings.value("SliceHistogram/smoothingRadius",HISTOGRAM_SMOOTHING_RADIUS).toInt();
	p_sliceHistogram.minimumHeightOfMaximums = settings.value("SliceHistogram/minimumHeightOfMaximums",HISTOGRAM_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM).toInt();
	p_sliceHistogram.derivativeSearchPercentage = settings.value("SliceHistogram/derivativeSearchPercentage",HISTOGRAM_DERIVATIVE_SEARCH_PERCENTAGE).toInt();
	p_sliceHistogram.minimumWidthOfIntervals = settings.value("SliceHistogram/minimumWidthOfIntervals",HISTOGRAM_MINIMUM_WIDTH_OF_INTERVALS).toInt();

	p_sectorHistogram.smoothingRadius = settings.value("SectorHistogram/smoothingRadius",HISTOGRAM_ANGULAR_SMOOTHING_RADIUS).toInt();
	p_sectorHistogram.minimumHeightOfMaximums = settings.value("SectorHistogram/minimumHeightOfMaximums",HISTOGRAM_ANGULAR_PERCENTAGE_OF_MINIMUM_HEIGHT_OF_MAXIMUM).toInt();
	p_sectorHistogram.derivativeSearchPercentage = settings.value("SectorHistogram/derivativeSearchPercentage",HISTOGRAM_ANGULAR_DERIVATIVE_SEARCH_PERCENTAGE).toInt();
	p_sectorHistogram.minimumWidthOfIntervals = settings.value("SectorHistogram/minimumWidthOfIntervals",HISTOGRAM_ANGULAR_MINIMUM_WIDTH_OF_INTERVALS).toInt();

	p_pithDetectionKnot.subWindowWidth = settings.value("PithDetectionKnot/subWindowWidth",NEIGHBORHOOD_WINDOW_WIDTH_KNOT).toInt();
	p_pithDetectionKnot.subWindowHeight = settings.value("PithDetectionKnot/subWindowHeight",NEIGHBORHOOD_WINDOW_HEIGHT_KNOT).toInt();
	p_pithDetectionKnot.pithShift = settings.value("PithDetectionKnot/pithShift",PITH_LAG_KNOT).toInt();
	p_pithDetectionKnot.smoothingRadius = settings.value("PithDetectionKnot/smoothingRadius",PITH_SMOOTHING_KNOT).toInt();
	p_pithDetectionKnot.minimumWoodPercentage = settings.value("PithDetectionKnot/minimumWoodPercentage",MIN_WOOD_PERCENTAGE_KNOT).toInt();
	p_pithDetectionKnot.useAscendingOrder = settings.value("PithDetectionKnot/useAscendingOrder",ASCENDING_ORDER_KNOT).toBool();
	p_pithDetectionKnot.extrapolationType = settings.value("PithDetectionKnot/extrapolationType",TKD::SLOPE_DIRECTION).toInt();
	p_pithDetectionKnot.percentageOfFirstValidSlicesToExtrapolate = settings.value("PithDetectionKnot/percentageOfFirstValidSlicesToExtrapolate",FIRST_VALID_SLICES_TO_EXTRAPOLATE_KNOT).toInt();
	p_pithDetectionKnot.percentageOfLastValidSlicesToExtrapolate = settings.value("PithDetectionKnot/percentageOfLastValidSlicesToExtrapolate",LAST_VALID_SLICES_TO_EXTRAPOLATE_KNOT).toInt();

	p_tangentialGenerator.useTrilinearInterpolation = settings.value("TangentialGenerator/useTrilinearInterpolation",true).toBool();

	p_pithProfile.smoothingRadius = settings.value("PithProfile/smoothingRadius",2).toInt();

	p_ellipticalHistograms.lowessBandWidth = settings.value("EllipticalHistograms/lowessBandWidth",33).toInt();
	p_ellipticalHistograms.lowessIqrCoefficient  = settings.value("EllipticalHistograms/lowessIqrCoefficient ",1).toInt();
	p_ellipticalHistograms.lowessPercentageOfFirstValidSlicesToExtrapolate = settings.value("EllipticalHistograms/lowessPercentageOfFirstValidSlicesToExtrapolate",5).toInt();
	p_ellipticalHistograms.lowessPercentageOfLastValidSlicesToExtrapolate = settings.value("EllipticalHistograms/lowessPercentageOfLastValidSlicesToExtrapolate",0).toInt();
	p_ellipticalHistograms.smoothingRadius = settings.value("EllipticalHistograms/smoothingRadius",0).toInt();

	Billon *_billon = 0;

	/********************************************************************/
	qDebug() << "2) Lecture du dicom...";

	QString folderName("plop");
	if ( folderName.isEmpty() )
	{
		qDebug() << "ERREUR : le répertoire spécifié n'existe pas.";
		return 1;
	}

	_billon = DicomReader::read( folderName );
	if ( !_billon )
	{
		qDebug() << "ERREUR : le chargement de l'image a échoué.";
		return 2;
	}

	/********************************************************************/
	qDebug() << "3) Calcul de la moelle du tronc...";

	qreal _treeRadius;
	computeBillonPith( *_billon, _treeRadius );

	if ( !_billon->hasPith() )
	{
		qDebug() << "ERREUR : la moelle du tronc n'a pas pu être calculée.";
		delete _billon;
		return 3;
	}

	/********************************************************************/
	qDebug() << "4) Détection des zones de nœuds...";

	KnotAreaDetector* _knotByWhorlDetector = new KnotByWhorlDetector;
	detectKnotsByWhorls( *_billon, _knotByWhorlDetector, _treeRadius );

	/********************************************************************/
	qDebug() << "5) Segmentation des zones de nœuds...";

	segmentKnots( *_billon,*_knotByWhorlDetector );

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

/****************************************************
 * Implémentation des fonctions appellées par le main
 * **************************************************/
void computeBillonPith( Billon& billon, qreal &treeRadius )
{
	 PithExtractorBoukadida _billonPithExtractor;

	_billonPithExtractor.setSubWindowWidth( p_pithDetectionBillon.subWindowWidth );
	_billonPithExtractor.setSubWindowHeight( p_pithDetectionBillon.subWindowHeight );
	_billonPithExtractor.setPithShift( p_pithDetectionBillon.pithShift );
	_billonPithExtractor.setSmoothingRadius( p_pithDetectionBillon.smoothingRadius );
	_billonPithExtractor.setMinWoodPercentage( p_pithDetectionBillon.minimumWoodPercentage );
	_billonPithExtractor.setIntensityInterval( Interval<int>( p_global.minIntensity, p_global.maxIntensity ) );
	_billonPithExtractor.setAscendingOrder( p_pithDetectionBillon.useAscendingOrder );
	_billonPithExtractor.setExtrapolation( static_cast<TKD::ExtrapolationType>(p_pithDetectionBillon.extrapolationType) );
	_billonPithExtractor.setFirstValidSlicesToExtrapolate( p_pithDetectionBillon.percentageOfFirstValidSlicesToExtrapolate );
	_billonPithExtractor.setLastValidSlicesToExtrapolate( p_pithDetectionBillon.percentageOfLastValidSlicesToExtrapolate );

	_billonPithExtractor.process( billon );

	treeRadius = BillonAlgorithms::restrictedAreaMeansRadius( billon,
															  p_restrictedArea.nbDirections,
															  p_restrictedArea.barkValue,
															  p_restrictedArea.minRadius*billon.n_cols/100.);
}

void detectKnotsByWhorls( const Billon &billon, KnotAreaDetector* knotByWhorlDetector, const qreal &treeRadius )
{
	knotByWhorlDetector->setIntensityInterval( Interval<int>(p_global.minIntensity,p_global.maxIntensity) );
	knotByWhorlDetector->setZMotionMin( p_global.minZMotion );
	knotByWhorlDetector->setTreeRadius( treeRadius );
	static_cast<KnotByWhorlDetector*>(knotByWhorlDetector)->setSliceHistogramParameters(
				p_sliceHistogram.smoothingRadius,
				p_sliceHistogram.minimumHeightOfMaximums,
				p_sliceHistogram.derivativeSearchPercentage,
				p_sliceHistogram.minimumWidthOfIntervals);

	const qreal coeffDegToSize = knotByWhorlDetector->pieChart().nbSectors()/360.;
	static_cast<KnotByWhorlDetector*>(knotByWhorlDetector)->setSectorHistogramsParameters(
				p_sectorHistogram.smoothingRadius*coeffDegToSize,
				p_sectorHistogram.minimumHeightOfMaximums,
				p_sectorHistogram.derivativeSearchPercentage,
				p_sectorHistogram.minimumWidthOfIntervals*coeffDegToSize);

	knotByWhorlDetector->execute( billon );
}

void segmentKnots( const Billon &billon, const KnotAreaDetector &detector )
{
	TangentialGenerator tangentialGenerator( p_global.minIntensity, p_tangentialGenerator.useTrilinearInterpolation );

	PithExtractorBoukadida knotPithExtractor;
	knotPithExtractor.setSubWindowWidth( p_pithDetectionKnot.subWindowWidth );
	knotPithExtractor.setSubWindowHeight( p_pithDetectionKnot.subWindowHeight );
	knotPithExtractor.setPithShift( p_pithDetectionKnot.pithShift );
	knotPithExtractor.setSmoothingRadius( p_pithDetectionKnot.smoothingRadius );
	knotPithExtractor.setMinWoodPercentage( p_pithDetectionKnot.minimumWoodPercentage );
	knotPithExtractor.setIntensityInterval( Interval<int>( p_global.minIntensity, p_global.maxIntensity ) );
	knotPithExtractor.setAscendingOrder( p_pithDetectionKnot.useAscendingOrder );
	knotPithExtractor.setExtrapolation( static_cast<TKD::ExtrapolationType>(p_pithDetectionKnot.extrapolationType) );
	knotPithExtractor.setFirstValidSlicesToExtrapolate( p_pithDetectionKnot.percentageOfFirstValidSlicesToExtrapolate );
	knotPithExtractor.setLastValidSlicesToExtrapolate( p_pithDetectionKnot.percentageOfLastValidSlicesToExtrapolate );

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
		if ( tangentialBillon->hasPith() )
		{
			knotPithProfile.construct( tangentialBillon->pith(), p_pithProfile.smoothingRadius );
			if ( knotPithProfile.size() )
			{
				knotEllipseRadiiHistogram.construct( *tangentialBillon,
													 knotPithProfile,
													 p_ellipticalHistograms.lowessBandWidth,
													 p_ellipticalHistograms.smoothingRadius,
													 p_ellipticalHistograms.lowessIqrCoefficient,
													 p_ellipticalHistograms.lowessPercentageOfFirstValidSlicesToExtrapolate,
													 p_ellipticalHistograms.lowessPercentageOfLastValidSlicesToExtrapolate );
			}
		}
	}

	return tangentialBillon;
}
