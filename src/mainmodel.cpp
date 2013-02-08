#include "inc/mainmodel.h"

#include "inc/billon.h"
#include "inc/piepart.h"
#include "inc/piechart.h"
#include "inc/slicehistogram.h"
#include "inc/sectorhistogram.h"
#include "inc/nearestpointshistogram.h"
#include "inc/intensitydistributionhistogram.h"
#include "inc/zmotiondistributionhistogram.h"
#include "inc/contourbillon.h"

#include "inc/billonalgorithms.h"
#include "inc/dicomreader.h"

MainModel::MainModel() : _billon(0), _componentBillon(0), _knotBillon(0),
	_pieChart(new PieChart(360)), _sliceHistogram(new SliceHistogram()), _sectorHistogram(new SectorHistogram()),
	_nearestPointsHistogram(new NearestPointsHistogram()), _intensityDistributionHistogram(new IntensityDistributionHistogram()),
	_zMotionDistributionHistogram(new ZMotionDistributionHistogram()), _contourBillon(new ContourBillon()),
	_currentSlice(0), _currentYSlice(0), _currentMaximum(0), _currentSector(0), _treeRadius(0)
{
}

MainModel::~MainModel()
{
	delete _contourBillon;
	delete _nearestPointsHistogram;
	delete _sectorHistogram;
	delete _pieChart;
	delete _sliceHistogram;
	if ( _knotBillon != 0 ) delete _knotBillon;
	if ( _componentBillon != 0 ) delete _componentBillon;
	if ( _billon != 0 ) delete _billon;
}

/*******************************
 * Public fonctions
 *******************************/

void MainModel::readBillon( const QString &folderName )
{
	if ( folderName.isEmpty() ) return;
	Billon *billon = DicomReader::read(folderName);
	if ( billon == 0 ) return;
	cleanAll();
	setBillon(billon);
}

void MainModel::setBillon( Billon *billon )
{
	if ( _billon != 0 )	delete _billon;
	_billon = billon;
}

void MainModel::cleanAll()
{
	if ( _billon != 0 )	delete _billon;
	if ( _componentBillon != 0 ) delete _componentBillon;
	if ( _knotBillon != 0 )	delete _knotBillon;

	_billon = _componentBillon = _knotBillon = 0;

	_sliceHistogram->clear();
	_sectorHistogram->clear();
	_nearestPointsHistogram->clear();
	_intensityDistributionHistogram->clear();
	_zMotionDistributionHistogram->clear();

	_contourBillon->clear();

	_currentSlice = 0;
	_currentYSlice = 0;
	_currentMaximum = 0;
	_currentSector = 0;
	_treeRadius = 133.33;
}

void MainModel::setCurrentSlice( const uint &sliceIndex )
{
	Q_ASSERT( _billon ? sliceIndex < _billon->n_slices : sliceIndex == 0 );
	_currentSlice = sliceIndex;
}

void MainModel::setCurrentYSlice( const uint &sliceIndex )
{
	Q_ASSERT( _billon ? sliceIndex < _billon->n_rows : sliceIndex == 0 );
	_currentYSlice = sliceIndex;
}

void MainModel::setCurrentMaximum( const uint &index )
{
	Q_ASSERT( _sliceHistogram->isEmpty() ? index == 0 : index < _sliceHistogram->nbMaximums() );
	_currentMaximum = index;
}

void MainModel::setCurrentSector( const uint &index )
{
	Q_ASSERT( _sectorHistogram->nbIntervals() ? index < _sectorHistogram->nbIntervals() : index == 0 );
	_currentSector = index;
}

void MainModel::updateTreeRadius( const uint &resolution, const int &threshold )
{
	_treeRadius = _billon ? BillonAlgorithms::restrictedAreaMeansRadius( *_billon, resolution, threshold )
						  : RESTRICTED_AREA_DEFAULT_RADIUS;
}
