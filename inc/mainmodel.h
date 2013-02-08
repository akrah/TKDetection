#ifndef MAINMODEL_H
#define MAINMODEL_H

#include "def/def_billon.h"

#include <QtGlobal>

class PieChart;
class SliceHistogram;
class SectorHistogram;
class NearestPointsHistogram;
class IntensityDistributionHistogram;
class ZMotionDistributionHistogram;
class ContourBillon;

class MainModel
{
public:
	MainModel();
	~MainModel();

public:
	void readBillon( const QString &folderName );
	void setBillon( Billon *billon );
	void cleanAll();

	void setCurrentSlice( const uint &sliceIndex );
	void setCurrentYSlice( const uint &sliceIndex );
	void setCurrentMaximum( const uint &index );
	void setCurrentSector( const uint &index );
	void updateTreeRadius( const uint &resolution, const int &threshold );



private:
	Billon *_billon;
	Billon *_componentBillon;
	Billon *_knotBillon;

	PieChart *_pieChart;
	SliceHistogram *_sliceHistogram;
	SectorHistogram *_sectorHistogram;
	NearestPointsHistogram *_nearestPointsHistogram;
	IntensityDistributionHistogram *_intensityDistributionHistogram;
	ZMotionDistributionHistogram *_zMotionDistributionHistogram;

	ContourBillon *_contourBillon;

	uint _currentSlice;
	uint _currentYSlice;
	uint _currentMaximum;
	uint _currentSector;
	qreal _treeRadius;
};

#endif // MAINMODEL_H
