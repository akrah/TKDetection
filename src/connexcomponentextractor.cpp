#include "inc/connexcomponentextractor.h"

#include "inc/marrow_def.h"
#include "inc/billon.h"

#include <armadillo>

#include <QList>
#include <QPair>

#include <QDebug>

namespace ConnexComponentExtractor
{

	namespace
	{
		/**
		 * \fn		int twoPassAlgorithm( arma::Slice &oldSlice, arma::Slice &currentSlice, arma::Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel )
		 * \brief	Algorithme de labelisation de composantes connexes en 3 dimensions
		 * \param	oldSlice Tranche précédente déjà labelisée
		 * \param	currentSlice Tranche courante où sont extraites les composantes connexes
		 * \param	labels Matrice 2D qui contiendra la tranche courante labelisée
		 * \param	connexComponentList Liste qui associe à un label la liste des coordonnées des points de sa composante connexe
		 * \param	k Numéro de la tranche courante dans l'image globale
		 * \param	nbLabel Nombre de labels déjà attribués
		 * \return	le dernier label attribué
		 */
		int twoPassAlgorithm( arma::Slice &oldSlice, arma::Slice &currentSlice, arma::Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold );
	}

	Billon * extractConnexComponents( Billon &billon, const int &minimumSize, const __billon_type__ &threshold )
	{
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;

		int nbLabel = 0;
		QMap<int, QList<iCoord3D> > connexComponentList;
		arma::Slice* labels = new arma::Slice(height, width);
		arma::Slice* oldSlice = new arma::Slice(height, width);
		oldSlice->fill(0);
		arma::Slice* tmp;

		//On parcours les tranches 1 par 1
		for ( unsigned int k=0 ; k<depth ; k++ )
		{
			nbLabel = twoPassAlgorithm((*oldSlice),billon.slice(k),(*labels),connexComponentList,k,nbLabel,threshold);
			tmp = oldSlice;
			oldSlice = labels;
			labels = tmp;
			tmp = 0;
		}

		delete labels;
		delete oldSlice;

		Billon* components = new Billon(width,height,depth);
		components->setVoxelSize(billon.voxelWidth(),billon.voxelHeight(),billon.voxelDepth());
		components->fill(0);

		QMapIterator<int, QList<iCoord3D> > iter(connexComponentList);
		iCoord3D currentCoord;
		int counter = 1;
		while (iter.hasNext())
		{
			iter.next();
			if ( iter.value().size() > minimumSize )
			{
				QListIterator<iCoord3D> coords(iter.value());
				while (coords.hasNext())
				{
					currentCoord = coords.next();
					components->at(currentCoord.y,currentCoord.x,currentCoord.z) = counter;
				}
				counter++;
			}
		}
		qDebug() << QObject::tr("Nombre de composantes = %1").arg(counter-1);
		components->setMinValue(0);
		components->setMaxValue(counter-1);

		return components;
	}

	Billon * extractBiggestConnexComponent( Billon &billon, const __billon_type__ &threshold )
	{
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;

		int nbLabel = 0;
		QMap<int, QList<iCoord3D> > connexComponentList;
		arma::Slice* labels = new arma::Slice(height, width);
		arma::Slice* oldSlice = new arma::Slice(height, width);
		oldSlice->fill(0);
		arma::Slice* tmp;

		//On parcours les tranches 1 par 1
		for ( unsigned int k=0 ; k<depth ; k++ )
		{
			nbLabel = twoPassAlgorithm((*oldSlice),billon.slice(k),(*labels),connexComponentList,k,nbLabel,threshold);
			tmp = oldSlice;
			oldSlice = labels;
			labels = tmp;
			tmp = 0;
		}

		delete labels;
		delete oldSlice;

		int bigestIndex, bigestSize;
		bigestIndex = bigestSize = 0;
		QMapIterator<int, QList<iCoord3D> > iterComponents(connexComponentList);
		while (iterComponents.hasNext())
		{
			iterComponents.next();
			if ( iterComponents.value().size() > bigestSize )
			{
				bigestSize = iterComponents.value().size();
				bigestIndex = iterComponents.key();
			}
		}

		Billon* components = new Billon(width,height,depth);
		components->setVoxelSize(billon.voxelWidth(),billon.voxelHeight(),billon.voxelDepth());
		components->setMinValue(0);
		components->setMaxValue(1);
		components->fill(0);

		QListIterator<iCoord3D> coords(connexComponentList[bigestIndex]);
		iCoord3D currentCoord;
		while (coords.hasNext())
		{
			currentCoord = coords.next();
			components->at(currentCoord.y,currentCoord.x,currentCoord.z) = 1;
		}

		return components;
	}

	Billon * extractConnexComponents( arma::Slice &currentSlice, const int &minimumSize, const __billon_type__ &threshold )
	{
		const uint width = currentSlice.n_cols;
		const uint height = currentSlice.n_rows;

		QMap<int, QList<iCoord2D> > connexComponentList;
		QMap<int, int> tableEquiv;
		QList<int> voisinage;
		uint j, i;
		int mini, nbLabel, label;

		arma::Slice labels(height, width);
		labels.fill(0);
		nbLabel = 0;
		//On parcourt une première fois la tranche
		for ( j=1 ; j<height ; j++)
		{
			for ( i=1 ; i<width-1 ; i++)
			{
				//Si on a un voxel
				if ( currentSlice.at(j,i) > threshold )
				{
					//On sauvegarde la valeur des voisins non nuls
					voisinage.clear();
					//Voisinage de la face courante
					if (labels.at(j,i-1)) voisinage.append(labels.at(j,i-1));
					if (labels.at(j-1,i-1))voisinage.append(labels.at(j-1,i-1));
					if (labels.at(j-1,i)) voisinage.append(labels.at(j-1,i));
					if (labels.at(j-1,i+1)) voisinage.append(labels.at(j-1,i+1));
					//Si ses voisins n'ont pas d'étiquette
					if ( voisinage.isEmpty() )
					{
						nbLabel++;
						labels.at(j,i) = nbLabel;
					}
					//Si ses voisins ont une étiquette
					else if ( voisinage.size() == 1 )
					{
						labels.at(j,i) = voisinage[0];
					}
					else
					{
						QListIterator<int> iterVoisin(voisinage);
						mini = iterVoisin.next();
						while(iterVoisin.hasNext())
						{
							mini = qMin(mini,iterVoisin.next());
						}
						labels.at(j,i) = mini;
						iterVoisin.toFront();
						while(iterVoisin.hasNext())
						{
							label = iterVoisin.next();
							if ( label != mini )
							{
								tableEquiv[label] = mini;
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
			while (tableEquiv.contains(value))
			{
				value = tableEquiv[value];
			}
			tableEquiv[iter.key()] = value;
		}
		for ( j=0 ; j<height ; j++ )
		{
			for ( i=0 ; i<width ; i++ )
			{
				label = labels.at(j,i);
				//Si on a un voxel
				if (label)
				{
					if (tableEquiv.contains(label))
					{
						labels.at(j,i) = tableEquiv[label];
						label = labels.at(j,i);
					}
					if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord2D>();
					connexComponentList[label].append(iCoord2D(i,j));
				}
			}
		}

		Billon *bigestComponentsInBillon = new Billon(width,height,1);
		bigestComponentsInBillon->setMinValue(0);
		bigestComponentsInBillon->fill(0);
		iCoord2D coord;
		int counter = 1;
		QMapIterator<int, QList<iCoord2D> > iterComponents(connexComponentList);
		while (iterComponents.hasNext())
		{
			iterComponents.next();
			if ( iterComponents.value().size() > minimumSize )
			{
				QListIterator<iCoord2D> coordIterator(iterComponents.value());
				while ( coordIterator.hasNext() )
				{
					coord = coordIterator.next();
					bigestComponentsInBillon->at(coord.y,coord.x,0) = counter;
				}
				counter++;
			}
		}
		bigestComponentsInBillon->setMaxValue(counter-1);

		return bigestComponentsInBillon;
	}

	Billon * extractBiggestConnexComponent( arma::Slice &currentSlice, const __billon_type__ &threshold )
	{
		const uint width = currentSlice.n_cols;
		const uint height = currentSlice.n_rows;

		QMap<int, QList<iCoord2D> > connexComponentList;
		QMap<int, int> tableEquiv;
		QList<int> voisinage;
		uint j, i;
		int mini, nbLabel, label;

		arma::Slice labels(height, width);
		labels.fill(0);
		nbLabel = 0;
		//On parcourt une première fois la tranche
		for ( j=1 ; j<height ; j++)
		{
			for ( i=1 ; i<width-1 ; i++)
			{
				//Si on a un voxel
				if ( currentSlice.at(j,i) > threshold )
				{
					//On sauvegarde la valeur des voisins non nuls
					voisinage.clear();
					//Voisinage de la face courante
					if (labels.at(j,i-1)) voisinage.append(labels.at(j,i-1));
					if (labels.at(j-1,i-1))voisinage.append(labels.at(j-1,i-1));
					if (labels.at(j-1,i)) voisinage.append(labels.at(j-1,i));
					if (labels.at(j-1,i+1)) voisinage.append(labels.at(j-1,i+1));
					//Si ses voisins n'ont pas d'étiquette
					if ( voisinage.isEmpty() )
					{
						nbLabel++;
						labels.at(j,i) = nbLabel;
					}
					//Si ses voisins ont une étiquette
					else if ( voisinage.size() == 1 )
					{
						labels.at(j,i) = voisinage[0];
					}
					else
					{
						QListIterator<int> iterVoisin(voisinage);
						mini = iterVoisin.next();
						while(iterVoisin.hasNext())
						{
							mini = qMin(mini,iterVoisin.next());
						}
						labels.at(j,i) = mini;
						iterVoisin.toFront();
						while(iterVoisin.hasNext())
						{
							label = iterVoisin.next();
							if ( label != mini )
							{
								tableEquiv[label] = mini;
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
			while (tableEquiv.contains(value))
			{
				value = tableEquiv[value];
			}
			tableEquiv[iter.key()] = value;
		}
		for ( j=0 ; j<height ; j++ )
		{
			for ( i=0 ; i<width ; i++ )
			{
				label = labels.at(j,i);
				//Si on a un voxel
				if (label)
				{
					if (tableEquiv.contains(label))
					{
						labels.at(j,i) = tableEquiv[label];
						label = labels.at(j,i);
					}
					if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord2D>();
					connexComponentList[label].append(iCoord2D(i,j));
				}
			}
		}

		int bigestIndex, bigestSize;
		bigestIndex = bigestSize = 0;
		QMapIterator<int, QList<iCoord2D> > iterComponents(connexComponentList);
		while (iterComponents.hasNext())
		{
			iterComponents.next();
			if ( iterComponents.value().size() > bigestSize )
			{
				bigestSize = iterComponents.value().size();
				bigestIndex = iterComponents.key();
			}
		}

		Billon *bigestComponentInBillon = new Billon(width,height,1);
		bigestComponentInBillon->setMinValue(0);
		bigestComponentInBillon->setMaxValue(1);
		bigestComponentInBillon->fill(0);
		if ( bigestSize > 0 )
		{
			QListIterator<iCoord2D> bigestComponentIterator(connexComponentList.value(bigestIndex));
			while ( bigestComponentIterator.hasNext() )
			{
				iCoord2D coord = bigestComponentIterator.next();
				bigestComponentInBillon->at(coord.y,coord.x,0) = 1;
			}
		}

		return bigestComponentInBillon;
	}


	namespace
	{
		int twoPassAlgorithm( arma::Slice &oldSlice, arma::Slice &currentSlice, arma::Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold )
		{
			const uint width = currentSlice.n_cols;
			const uint height = currentSlice.n_rows;

			QMap<int, int> tableEquiv;
			QList<int> voisinage;
			uint j, i;
			int mini, oldStart, sup, inf, label;
			bool isOld;
			labels.fill(0);
			//On parcourt une première fois la tranche
			for ( j=1 ; j<height-1 ; j++)
			{
				for ( i=1 ; i<width-1 ; i++)
				{
					//Si on a un voxel
					if ( currentSlice.at(j,i) > threshold )
					{
						//On sauvegarde la valeur des voisins non nuls
						voisinage.clear();
						//Voisinage de la face courante
						if (labels.at(j,i-1)) voisinage.append(labels.at(j,i-1));
						if (labels.at(j-1,i-1))voisinage.append(labels.at(j-1,i-1));
						if (labels.at(j-1,i)) voisinage.append(labels.at(j-1,i));
						if (labels.at(j-1,i+1)) voisinage.append(labels.at(j-1,i+1));
						oldStart = voisinage.size();
						//Voisinage de la face arrière
						if (oldSlice.at(j-1,i-1)) voisinage.append(oldSlice.at(j-1,i-1));
						if (oldSlice.at(j-1,i)) voisinage.append(oldSlice.at(j-1,i));
						if (oldSlice.at(j-1,i+1)) voisinage.append(oldSlice.at(j-1,i+1));
						if (oldSlice.at(j,i-1)) voisinage.append(oldSlice.at(j,i-1));
						if (oldSlice.at(j,i)) voisinage.append(oldSlice.at(j,i));
						if (oldSlice.at(j,i+1)) voisinage.append(oldSlice.at(j,i+1));
						if (oldSlice.at(j+1,i-1)) voisinage.append(oldSlice.at(j+1,i-1));
						if (oldSlice.at(j+1,i)) voisinage.append(oldSlice.at(j+1,i));
						if (oldSlice.at(j+1,i+1)) voisinage.append(oldSlice.at(j+1,i+1));
						//Si ses voisins n'ont pas d'étiquette
						if ( voisinage.isEmpty() )
						{
							nbLabel++;
							labels.at(j,i) = nbLabel;
						}
						//Si ses voisins ont une étiquette
						else
						{
							QListIterator<int> iterVoisin(voisinage);
							mini = iterVoisin.next();
							while(iterVoisin.hasNext())
							{
								mini = qMin(mini,iterVoisin.next());
							}
							//Attribution de la valeur minimale au voxel courant
							labels.at(j,i) = mini;
							isOld = connexComponentList.contains(mini);
							//Mise à jour de la table d'équivalence pour la face courante
							//et fusion des liste de sommets si un voxel fusionne des composantes connexes antérieures
							for ( int ind=0 ; ind<oldStart ; ind++ )
							{
								if ( voisinage[ind] > mini )
								{
									tableEquiv[voisinage[ind]] = mini;
									if (isOld && connexComponentList.contains(voisinage[ind]))
									{
										connexComponentList[mini].append(connexComponentList.take(voisinage[ind]));
										//tableEquiv[voisinage[ind]] = mini; EST DEJA FAIT AU DESSUS DU IF
									}
								}
							}
							if ( isOld )
							{
								for ( int ind=oldStart ; ind<voisinage.size() ; ind++ )
								{
									if ( voisinage[ind] != mini )
									{
										if ( mini>voisinage[ind] )
										{
											sup = mini;
											inf = voisinage[ind];
										} else
										{
											sup = voisinage[ind];
											inf = mini;
										}
										tableEquiv[sup] = inf;
										connexComponentList[inf].append(connexComponentList.take(sup));
									}
								}
							}
						}
					}
				}
			}
			//Résolution des chaines dans la table d'équivalence
			QMapIterator<int, int> iter(tableEquiv);
			while (iter.hasNext())
			{
				iter.next();
				if (tableEquiv.contains(iter.value()))
				{
					tableEquiv[iter.key()] = tableEquiv[iter.value()];
				}
			}
			for ( j=0 ; j<height ; j++ )
			{
				for ( i=0 ; i<width ; i++ )
				{
					label = labels.at(j,i);
					//Si on a un voxel
					if (label)
					{
						if (tableEquiv.contains(label))
						{
							labels.at(j,i) = tableEquiv[label];
							label = labels.at(j,i);
						}
						if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord3D>();
						connexComponentList[label].append(iCoord3D(i,j,k));
					}
				}
			}
			return nbLabel;
		}
	}
}
