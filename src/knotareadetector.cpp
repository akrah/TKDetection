#include "inc/knotareadetector.h"

#include "inc/coordinate.h"

#include <armadillo>

#include <QDebug>

KnotAreaDetector::KnotAreaDetector() : _binarizationThreshold(100), _maximumConnectedComponentDistance(20), _minimumConnectedComponentSize(0)
{
}

void KnotAreaDetector::execute( const Slice &accumulationSlice )
{
	// Initialize supportingAreaList to an empty list
	_supportingAreaVector.clear();

	// Binarized version of accumulationSlice with _binarizationThreshold threshold
	Slice labelledSlice( accumulationSlice.n_rows, accumulationSlice.n_cols, arma::fill::zeros );
	// List of connected components of thImage
	QMap<int, QList<iCoord2D> > ccList;
	// Compute thImage as the binarized image of imageAcc using tBin
	computeLabelledImage( accumulationSlice, labelledSlice, ccList );

	// Sort all pixels of imageAcc by decreasing value order into sortedPixels
	arma::Col<uint> sortedPixelIndex = arma::sort_index(arma::vectorise(accumulationSlice), "descend");

	// The support of each detected knot area
	QMap<int, QList<iCoord2D> > supportingAreaMap;

	// List of processed connected components of ccList
	QList<int> ccProcessedList;

	int pixelLinearIter, pixelLinearPos, cc, pixCoordX, pixCoordY, ccMin;
	qreal minDist;

	pixelLinearIter = 0;

	while( ccList.size() > 0 )
	{
		pixelLinearPos = sortedPixelIndex(pixelLinearIter++);
		pixCoordX = pixelLinearPos/accumulationSlice.n_rows;
		pixCoordY = pixelLinearPos%accumulationSlice.n_rows;

		cc = labelledSlice( pixCoordY, pixCoordX );

		if ( !ccProcessedList.contains(cc) )
		{
			ccProcessedList.append(cc);
			findNearestConnectedComponent( ccList[cc], supportingAreaMap, ccMin, minDist );
			if ( ccMin != -1 && minDist <= _maximumConnectedComponentDistance )
			{
				supportingAreaMap[ccMin].append(ccList.take(cc));
			}
			else
			{
				supportingAreaMap[cc] = ccList.take(cc);
			}
		}
	}

	QMapIterator<int, QList<iCoord2D> > supportingAreaMapIter(supportingAreaMap);
	QRect supportingArea;
	while ( supportingAreaMapIter.hasNext() )
	{
		supportingAreaMapIter.next();
		if ( supportingAreaMapIter.value().size() >= _minimumConnectedComponentSize )
		{
			QListIterator<iCoord2D> supportingAreaIter(supportingAreaMapIter.value());
			if ( supportingAreaIter.hasNext() )
			{
				const iCoord2D &firstCoord = supportingAreaIter.next();
				supportingArea.setCoords( firstCoord.x, firstCoord.y, firstCoord.x, firstCoord.y );
				while ( supportingAreaIter.hasNext() )
				{
					const iCoord2D &coord = supportingAreaIter.next();
					supportingArea.setCoords( qMin(supportingArea.left(), coord.x), qMin(supportingArea.top(), coord.y), qMax(supportingArea.right(), coord.x), qMax(supportingArea.bottom(), coord.y) );
				}
				_supportingAreaVector.append(supportingArea);
			}
		}
	}
}

void KnotAreaDetector::computeLabelledImage( const Slice &accumulationSlice, Slice &labelledSlice, QMap<int, QList<iCoord2D> > &ccList )
{
	QMap<int, int> tableEquiv;
	QList<int> voisinage;
	int mini, nbLabels, indiceVoisin;
	__billon_type__ label;

	//On parcours une première fois la tranche
	nbLabels = 0;
	for ( uint j=1 ; j<accumulationSlice.n_rows-1 ; ++j )
	{
		for ( uint i=1 ; i<accumulationSlice.n_cols-1 ; ++i )
		{
			//Si on a un voxel
			if ( accumulationSlice(j,i) > _binarizationThreshold )
			{
				//On sauvegarde la valeur des voisins non nuls
				voisinage.clear();
				//Voisinage de la face courante
				if (labelledSlice(j,i-1)) voisinage.append(labelledSlice(j,i-1));
				if (labelledSlice(j-1,i-1))voisinage.append(labelledSlice(j-1,i-1));
				if (labelledSlice(j-1,i)) voisinage.append(labelledSlice(j-1,i));
				if (labelledSlice(j-1,i+1)) voisinage.append(labelledSlice(j-1,i+1));
				//Si ses voisins n'ont pas d'étiquette
				if ( voisinage.isEmpty() )
				{
					nbLabels++;
					labelledSlice(j,i) = nbLabels;
				}
				//Si ses voisins ont une étiquette
				else
				{
					mini = voisinage[0];
					for ( indiceVoisin=1 ; indiceVoisin<voisinage.size() ; ++indiceVoisin )
					{
						mini = qMin(mini,voisinage[indiceVoisin]);
					}
					//Attribution de la valeur minimale au voxel courant
					labelledSlice(j,i) = mini;
					//Mise à jour de la table d'équivalence pour la face courante
					//et fusion des liste de sommets si un voxel fusionne des composantes connexes antérieures
					for ( indiceVoisin=0 ; indiceVoisin<voisinage.size() ; ++indiceVoisin )
					{
						if ( voisinage[indiceVoisin] > mini )
						{
							tableEquiv[voisinage[indiceVoisin]] = mini;
						}
					}
				}
			}
		}
	}
	//Résolution des chaines dans la table d'équivalence
	QMapIterator<int, int> iter(tableEquiv);
	int value;
	while (iter.hasNext())
	{
		iter.next();
		value = iter.value();
		if (tableEquiv.contains(value))
		{
			do {
				value = tableEquiv[value];
			} while ( tableEquiv.contains(value) );
			tableEquiv[iter.key()] = value;
		}
	}
	for ( uint j=0 ; j<accumulationSlice.n_rows ; ++j )
	{
		for ( uint i=0 ; i<accumulationSlice.n_cols ; ++i )
		{
			label = labelledSlice(j,i);
			//Si on a un voxel
			if (label)
			{
				if (tableEquiv.contains(label))
				{
					labelledSlice(j,i) = tableEquiv[label];
					label = labelledSlice(j,i);
				}
				if (!ccList.contains(label)) ccList[label] = QList<iCoord2D>();
				ccList[label].append(iCoord2D(i,j));
			}
		}
	}
}

void KnotAreaDetector::findNearestConnectedComponent( const QList<iCoord2D> &currentCC, const QMap<int, QList<iCoord2D> > &supportingAreaMap, int &ccMin, qreal &minDist )
{
	qreal distance;
	minDist = 999999;
	ccMin = -1;

	QMapIterator< int, QList<iCoord2D> > supportingAreaMapIter(supportingAreaMap);
	while ( supportingAreaMapIter.hasNext() )
	{
		supportingAreaMapIter.next();
		int currentCCIndex = supportingAreaMapIter.key();
		QListIterator<iCoord2D> currentSupportingAreaIter(supportingAreaMapIter.value());
		while ( currentSupportingAreaIter.hasNext() )
		{
			const iCoord2D &comparedCoord = currentSupportingAreaIter.next();
			QListIterator<iCoord2D> currentCCIter(currentCC);
			while ( currentCCIter.hasNext() )
			{
				const iCoord2D &currentCoord = currentCCIter.next();
				distance = comparedCoord.euclideanDistance(currentCoord);
				if ( distance < minDist )
				{
					minDist = distance;
					ccMin = currentCCIndex;
				}
			}
		}
	}
}

const __billon_type__ &KnotAreaDetector::binarizationThreshold() const
{
	return _binarizationThreshold;
}
const qreal &KnotAreaDetector::maximumConnectedComponentDistance() const
{
	return _maximumConnectedComponentDistance;
}

const uint &KnotAreaDetector::minimumConnectedComponentSize() const
{
	return _minimumConnectedComponentSize;
}

const QVector<QRect> &KnotAreaDetector::supportingAreaVector() const
{
	return _supportingAreaVector;
}

bool KnotAreaDetector::hasSupportingAreas() const
{
	return !_supportingAreaVector.isEmpty();
}

void KnotAreaDetector::setBinarizationThreshold( const __billon_type__ &newThreshold )
{
	_binarizationThreshold = newThreshold;
}

void KnotAreaDetector::setMaximumConnectedComponentDistance( const qreal &newDistance )
{
	_maximumConnectedComponentDistance = newDistance;
}

void KnotAreaDetector::setMinimumConnectedComponentSize( const uint &size )
{
	_minimumConnectedComponentSize = size;
}
