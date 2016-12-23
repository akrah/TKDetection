#include <QCoreApplication>
#include <QTextCodec>
#include <QSettings>

#include <QxtCommandOptions>
#include <QFile>
#include <QDir>
#include <QXmlStreamWriter>

#include "def/def_billon.h"
#include "inc/billon.h"

#include "inc/dicomreader.h"
#include "inc/pithextractorboukadida.h"
#include "inc/billonalgorithms.h"
#include "inc/detection/knotbywhorldetector.h"
#include "inc/detection/knotbyzmotionmapdetector.h"

#include "inc/segmentation/tangentialgenerator.h"
#include "inc/segmentation/pithprofile.h"
#include "inc/segmentation/ellipseradiihistogram.h"

#include "inc/options.h"

#include "inc/v3dexport.h"

#define DEFAULT_OUTPUT_DIRNAME		"TKDShell_Output"
#define DEFAULT_PARAMS_FILENAME		"tkd_shell.ini"
#define DEFAULT_DETECTION_METHOD	"whorls"
#define DEFAULT_KNOTFILE_BASE		"knot%1.sdp"
#define DEFAULT_GLOBALFILENAME		"global.v3d"

/**************************************************
 * Déclaration des fonctions appellées dans le main
 * ************************************************/
void computeBillonPith(
		Billon &billon,
		qreal &treeRadius );

void detectKnotsByWhorls(
		const Billon &billon,
		KnotAreaDetector* knotByWhorlDetector,
		const qreal &treeRadius );

void detectKnotsByZMotionMap(
		const Billon &billon,
		KnotAreaDetector* knotByWhorlDetector,
		const qreal &treeRadius );

void segmentAndExportKnots(
		const Billon &billon,
		const QString &outputDirName,
		const KnotAreaDetector &detector );

Billon* segmentKnotArea(
		const Billon &billon,
		const KnotArea &supportingArea,
		const PieChart& pieChart,
		TangentialGenerator &tangentialGenerator,
		PithExtractorBoukadida& knotPithExtractor,
		PithProfile &knotPithProfile,
		EllipseRadiiHistogram &knotEllipseRadiiHistogram);

void exportSegmentedKnotToSDP(
		const Billon &tangentialBillon,
		const TangentialGenerator &tangentialGenerator,
		const uint &knotIndex,
		const QDir &outputDir,
		const PithProfile& knotPithProfile,
		const EllipseRadiiHistogram& knotEllipseRadiiHistogram );


/*********************
 * Fonciton principale
 * *******************/
int main( int argc, char *argv[] )
{
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QCoreApplication app(argc,argv);
	app.setApplicationName("TKDetection");
	app.setApplicationVersion(QObject::tr("Version Shell 1.0"));


	/********************************************************************/
	// Gestion des arguments de la ligne de commande
	QxtCommandOptions options;
	options.setParamStyle(QxtCommandOptions::Space);

	options.addSection(QObject::tr("Obligatoires"));
	options.add("input", QObject::tr("Image DICOM du billon à segmenter"),QxtCommandOptions::ValueRequired);
	options.alias("input","i");

	options.addSection(QObject::tr("Optionnels"));
	options.add("output", QObject::tr("Nom du répertoire qui contiendra les fichiers des nœuds segmentés au format V3D et PGM3D (Défaut : %1)").arg(DEFAULT_OUTPUT_DIRNAME),QxtCommandOptions::ValueRequired);
	options.alias("output","o");
	options.add("params", QObject::tr("Fichier contenant l'ensemble des paramètres des différentes étapes au format INI (Défaut : %1)").arg(DEFAULT_PARAMS_FILENAME),QxtCommandOptions::ValueRequired);
	options.alias("params","p");
	options.add("method", QObject::tr("Choix de la méthode de détection des zones de nœuds. Peut être \"whorls\" ou \"zMotionMap\"  (Défaut : %1)").arg(DEFAULT_DETECTION_METHOD),QxtCommandOptions::ValueRequired);
	options.alias("method","m");

	options.addSection(QObject::tr("Autres"));
	options.add("help", QObject::tr("Afficher ce texte"),QxtCommandOptions::NoValue);
	options.alias("help", "h");

	options.parse(QCoreApplication::arguments());
	if ( !options.parameters().size() || options.count("help") || options.showUnrecognizedWarning() )
	{
		std::cout << QCoreApplication::applicationName().toStdString() << std::endl
				  << QCoreApplication::applicationVersion().toStdString() << std::endl;
		options.showUsage();
		return -1;
	}

	QString inputFileName;
	if ( !options.count("input") )
	{
		std::cout << QObject::tr("ERREUR : vous devez spécifier le répertoire contenant une image DICOM à segmenter").toStdString() << std::endl;
		options.showUsage();
		return -1;
	}
	inputFileName = options.value("input").toString();

	QString paramsFileName = options.count("params") ? options.value("params").toString() : DEFAULT_PARAMS_FILENAME;
	if ( !QFile(paramsFileName).exists() )
	{
		std::cout << QObject::tr("ERREUR : le fichier de paramètres %1 n'a pas été trouvé").arg(paramsFileName).toStdString() << std::endl;
		options.showUsage();
		return -1;
	}

	QString detectionMethod = options.count("method") ? options.value("method").toString() : DEFAULT_DETECTION_METHOD;
	if ( detectionMethod != "whorls" && detectionMethod != "zMotionMap" )
	{
		std::cout << QObject::tr("ERREUR : la méthode de détection des zones de nœuds '%1' n'existe pas. Elle peut valoir 'whorls' ou 'zMotionMap'").arg(detectionMethod).toStdString() << std::endl;
		options.showUsage();
		return -1;
	}

	QString outputDirName = options.count("output") ? options.value("output").toString() : DEFAULT_OUTPUT_DIRNAME;
	if ( QDir(outputDirName).exists() )
	{
//		std::string eraseExistingOutput;
		std::cout << QObject::tr("ATTENTION : le répertoire de sortie \"%1\" existe déjà. Êtes-vous sûr de vouloir écraser les fichiers présents dans ce répertoire ? (y/n) ").arg(outputDirName).toStdString();
//		std::cin >> eraseExistingOutput;
//		if ( eraseExistingOutput == "n" )
//		{
//			std::cout << QObject::tr("C'est bien ce qu'il me semblait... J'ai bien fait de demander !").toStdString() << std::endl;
//			return -1;
//		}
//		else
			std::cout << QObject::tr("Ok, le contenu du répertoire sera écrasé !").toStdString() << std::endl;
	}

	/********************************************************************/
	std::cout << "1) Lecture des paramètres..." << std::endl;
	QSettings settings(paramsFileName,QSettings::IniFormat);

	QStringList test = settings.allKeys();

	p_global.minIntensity = settings.value("Global/minIntensity",MINIMUM_INTENSITY).toInt();
	p_global.maxIntensity = settings.value("Global/maxIntensity",MAXIMUM_INTENSITY).toInt();
	p_global.minZMotion = settings.value("Global/minZMotion",MINIMUM_Z_MOTION).toInt();

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

	p_zMotionMap.binarizationThreshold = settings.value("ZMotionMap/binarizationThreshold",ZMOTIONMAP_BINARIZATION_THRESHOLD).toInt();
	p_zMotionMap.maximumConnectedComponentDistance = settings.value("ZMotionMap/maximumConnectedComponentDistance",ZMOTIONMAP_MAXIMUM_CONNECTED_COMPONENT_DISTANCE).toInt();
	p_zMotionMap.minimumConnectedComponentSize = settings.value("ZMotionMap/minimumConnectedComponentDistance",ZMOTIONMAP_MINIMUM_CONNECTED_COMPONENT_SIZE).toInt();

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

	return 0;

	Billon *_billon = 0;

	/********************************************************************/
	std::cout << "2) Lecture du dicom..." << std::endl;

	QDir folder(inputFileName);
	if ( !folder.exists() )
	{
		std::cout << QObject::tr("ERREUR : le répertoire DICOM \"%1\" n'existe pas.").arg(folder.path()).toStdString() << std::endl;
		return -1;
	}

	_billon = DicomReader::read( folder.path() );
	if ( !_billon )
	{
		std::cout << "ERREUR : le chargement de l'image a échoué." << std::endl;
		return -1;
	}

	/********************************************************************/
	std::cout << "3) Calcul de la moelle du tronc..." << std::endl;

	qreal _treeRadius;
	computeBillonPith( *_billon, _treeRadius );

	if ( !_billon->hasPith() )
	{
		std::cout << "ERREUR : la moelle du tronc n'a pas pu être calculée." << std::endl;
		delete _billon;
		return -1;
	}

	/********************************************************************/
	std::cout << "4) Détection des zones de nœuds..." << std::endl;

	KnotAreaDetector* _knotAreaDetector;

	if ( detectionMethod == "whorls" )
	{
		_knotAreaDetector = new KnotByWhorlDetector;
		detectKnotsByWhorls( *_billon, _knotAreaDetector, _treeRadius );
	}
	else
	{
		_knotAreaDetector = new KnotByZMotionMapDetector;
		detectKnotsByZMotionMap( *_billon, _knotAreaDetector, _treeRadius );
	}

	/********************************************************************/
	std::cout << "5) Segmentation et exportation des zones de nœuds..." << std::endl;

	segmentAndExportKnots( *_billon,outputDirName,*_knotAreaDetector );

	/********************************************************************/
	std::cout << std::endl << "Fini !" << std::endl;

	delete _knotAreaDetector;
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
	knotByWhorlDetector->setSectorNumber( TWO_PI*treeRadius );

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

void detectKnotsByZMotionMap( const Billon &billon, KnotAreaDetector* knotByZMotionMapDetector, const qreal &treeRadius )
{
	knotByZMotionMapDetector->setIntensityInterval( Interval<int>(p_global.minIntensity,p_global.maxIntensity) );
	knotByZMotionMapDetector->setZMotionMin( p_global.minZMotion );
	knotByZMotionMapDetector->setTreeRadius( treeRadius );
	knotByZMotionMapDetector->setSectorNumber( TWO_PI*treeRadius );
	static_cast<KnotByZMotionMapDetector*>(knotByZMotionMapDetector)->setBinarizationThreshold( p_zMotionMap.binarizationThreshold );
	static_cast<KnotByZMotionMapDetector*>(knotByZMotionMapDetector)->setMaximumConnectedComponentDistance( p_zMotionMap.maximumConnectedComponentDistance );
	static_cast<KnotByZMotionMapDetector*>(knotByZMotionMapDetector)->setMinimumConnectedComponentSize( p_zMotionMap.minimumConnectedComponentSize );

	knotByZMotionMapDetector->execute( billon );
}

void segmentAndExportKnots( const Billon &billon, const QString &outputDirName, const KnotAreaDetector &detector )
{
	QDir outputDir(outputDirName);
	outputDir.mkpath(".");

	QFile globalFile(outputDir.filePath(DEFAULT_GLOBALFILENAME));
	if ( !globalFile.open(QIODevice::WriteOnly) )
	{
		std::cout << QObject::tr("ERREUR : le fichier global \"%1\" n'a pas pu être créé.").arg(globalFile.fileName()).toStdString() << std::endl;
		return;
	}

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
	PithProfile knotPihtProfile;
	EllipseRadiiHistogram knotEllipseRadiiHistogram;

	const int nbKnotAreas = detector.knotAreas().size();
	int i = 0;

	QXmlStreamWriter stream(&globalFile);
	V3DExport::init(globalFile,stream);

	V3DExport::appendTags( stream, billon );
	V3DExport::appendBillonPith( stream, billon );


	QVectorIterator<KnotArea> supportingAreasIter(detector.knotAreas());
	while ( supportingAreasIter.hasNext() )
	{
		std::cout << "    Noeud " << ++i << " / " << nbKnotAreas << " ..." << std::endl;
		const KnotArea &supportingArea = supportingAreasIter.next();
		tangentialBillon = segmentKnotArea( billon, supportingArea, detector.pieChart(), tangentialGenerator,
											knotPithExtractor, knotPihtProfile, knotEllipseRadiiHistogram );
		if ( tangentialBillon )
		{
			V3DExport::appendKnotArea( stream, i, *tangentialBillon, tangentialGenerator, knotPihtProfile, knotEllipseRadiiHistogram );
			exportSegmentedKnotToSDP( *tangentialBillon, tangentialGenerator, i, outputDir, knotPihtProfile, knotEllipseRadiiHistogram );
			std::cout << "        -> OK !" << std::endl;
			delete tangentialBillon;
		}
		else
		{
			std::cout << "        -> NONNNNNNNNNNNNNNNNNNNNNNNNNNN !!!" << std::endl;
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
						 PithExtractorBoukadida& knotPithExtractor,
						 PithProfile &knotPithProfile, EllipseRadiiHistogram &knotEllipseRadiiHistogram )
{
	Billon* knotBillon = 0;

	const Interval<uint> &sliceInterval = supportingArea.sliceInterval();
	const Interval<uint> &sectorInterval = supportingArea.sectorInterval();

	tangentialGenerator.updateIntervals( billon, sliceInterval, sectorInterval, pieChart );

	knotBillon = tangentialGenerator.execute( billon );

	if ( knotBillon )
	{
		knotPithExtractor.process(*knotBillon,true);
		if ( knotBillon->hasPith() )
		{
			knotPithProfile.construct( knotBillon->pith(), p_pithProfile.smoothingRadius );
			if ( knotPithProfile.size() )
			{
				knotEllipseRadiiHistogram.construct( *knotBillon,
													 knotPithProfile,
													 p_ellipticalHistograms.lowessBandWidth/100.,
													 p_ellipticalHistograms.smoothingRadius,
													 p_ellipticalHistograms.lowessIqrCoefficient,
													 p_ellipticalHistograms.lowessPercentageOfFirstValidSlicesToExtrapolate,
													 p_ellipticalHistograms.lowessPercentageOfLastValidSlicesToExtrapolate );
			}
		}
	}

	return knotBillon;
}

void exportSegmentedKnotToSDP( const Billon &tangentialBillon, const TangentialGenerator &tangentialGenerator,
							   const uint &knotIndex, const QDir &outputDir, const PithProfile &knotPithProfile,
							   const EllipseRadiiHistogram &knotEllipseRadiiHistogram )
{
	QFile knotFile(outputDir.filePath(QString(DEFAULT_KNOTFILE_BASE).arg(knotIndex)));
	if ( !knotFile.open(QIODevice::Append) )
	{
		std::cout << QObject::tr("ERREUR : le fichier du nœud %1 \"%2\" n'a pas pu être créé.").arg(knotIndex).arg(knotFile.fileName()).toStdString() << std::endl;
		return;
	}

	// Recherche des coordonnées min et max de la zone de nœud en x et y
	const QVector3D &origin = tangentialGenerator.origin();
	const QVector3D destLeftCoord = tangentialGenerator.rotate( QVector3D(0, 0, tangentialBillon.n_slices) );
	const QVector3D destRightCoord = tangentialGenerator.rotate( QVector3D(0, tangentialGenerator.width()-1, tangentialBillon.n_slices) );

	const iCoord3D minCoord(
				qMin(origin.x(),qMin(destLeftCoord.x(),destRightCoord.x())),
				qMin(origin.y(),qMin(destLeftCoord.y(),destRightCoord.y())),
				tangentialGenerator.currentSliceInterval().min());

	// coord maximum
	const iCoord3D maxCoord(
				qMax(origin.x(),qMax(destLeftCoord.x(),destRightCoord.x())),
				qMax(origin.y(),qMax(destLeftCoord.y(),destRightCoord.y())),
				tangentialGenerator.currentSliceInterval().max());

	QTextStream stream(&knotFile);
	stream << "SDP" << endl;
	stream << "## Position de le boite englobante" << endl;
	stream << "# " << minCoord.x << " " << minCoord.y << " " << minCoord.z << endl;
	stream << "## Dimension de la boite englobante" << endl;
	stream << "# " << maxCoord.x-minCoord.x << " " << maxCoord.y-minCoord.y << " " << maxCoord.z-minCoord.z << endl;
	stream << "## Dimension des voxels" << endl;
	stream << "# " << tangentialBillon.voxelWidth() << ' ' << tangentialBillon.voxelHeight() << ' ' << tangentialBillon.voxelDepth() << endl;
	stream << endl;

	const uint &nbSlices = tangentialBillon.n_slices;
	const qreal voxelRatio = tangentialBillon.voxelWidth()/tangentialBillon.voxelHeight();

	QVector3D destination;
	qreal a,b,x,y,xStart,xEnd,ellipticityRate,ellipseXCenter,ellipseYCenter;

	for ( uint k=0 ; k<nbSlices ; ++k )
	{
		ellipticityRate = voxelRatio / knotPithProfile[k];
		ellipseXCenter = tangentialBillon.pithCoord(k).x;
		ellipseYCenter = tangentialBillon.pithCoord(k).y;

		// Draw knot contour
		a = knotEllipseRadiiHistogram[k];
		b = a*ellipticityRate;
		for ( y=0 ; y<=b ; ++y )
		{
			xEnd = qSqrt( (1-qPow(y/b,2)) * qPow(a,2) );
			xStart = -xEnd;
			for ( x=xStart ; x<=xEnd ; ++x )
			{
				destination = tangentialGenerator.rotate( iCoord3D(ellipseXCenter+x,ellipseYCenter+y,k) );
				stream << destination.x() << " " << destination.y() << " " << destination.z() << endl;
				destination = tangentialGenerator.rotate( iCoord3D(ellipseXCenter+x,ellipseYCenter-y,k) );
				stream << destination.x() << " " << destination.y() << " " << destination.z() << endl;
			}
		}
	}
}
