#include "inc/connexcomponentextractor.h"

#include "inc/billon.h"

namespace ConnexComponentExtractor
{

	namespace
	{
		/**
		 * \fn		int twoPassAlgorithm( Slice &oldSlice, Slice &currentSlice, Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel )
		 * \brief	Algorithme de labelisation de composantes connexes en 3 dimensions
		 * \param	oldSlice Tranche précédente déjà labelisée
		 * \param	currentSlice Tranche courante où sont extraites les composantes connexes
		 * \param	labels Matrice 2D qui contiendra la tranche courante labelisée
		 * \param	connexComponentList Liste qui associe à un label la liste des coordonnées des points de sa composante connexe
		 * \param	k Numéro de la tranche courante dans l'image globale
		 * \param	nbLabel Nombre de labels déjà attribués
		 * \return	le dernier label attribué
		 */
		int twoPassAlgorithm( const Slice &oldSlice, const Slice &currentSlice, Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold );
	}

	Billon * extractConnexComponents( const Billon &billon, const int &minimumSize, const int &threshold )
	{
		Billon* components = new Billon(billon);
		extractConnexComponents(*components,billon,minimumSize,threshold);
		return components;
	}

	void extractConnexComponents( Billon &resultBillon, const Billon & billon, const int & minimumSize, const int & threshold )
	{
		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;

		int nbLabel = 0;
		QMap<int, QList<iCoord3D> > connexComponentList;
		Slice* labels = new Slice(height, width);
		Slice* oldSlice = new Slice(height, width);
		oldSlice->fill(0);
		Slice* tmp;

		//On parcours les tranches 1 par 1
		for ( unsigned int k=0 ; k<depth ; ++k )
		{
			nbLabel = twoPassAlgorithm((*oldSlice),billon.slice(k),(*labels),connexComponentList,k,nbLabel,threshold);
			tmp = oldSlice;
			oldSlice = labels;
			labels = tmp;
			tmp = 0;
		}

		delete labels;
		delete oldSlice;

		QMap<int, QList<iCoord3D> >::ConstIterator iterComponents;
		QList<iCoord3D>::ConstIterator iterCoords;
		int counter = 1;
		resultBillon.fill(0);
		for ( iterComponents = connexComponentList.constBegin() ; iterComponents != connexComponentList.constEnd() ; ++iterComponents )
		{
			if ( iterComponents.value().size() > minimumSize )
			{
				for ( iterCoords = iterComponents.value().constBegin() ; iterCoords != iterComponents.value().constEnd() ; ++iterCoords )
				{
					resultBillon((*iterCoords).y,(*iterCoords).x,(*iterCoords).z) = counter;
				}
				++counter;
			}
		}
		qDebug() << QObject::tr("Nombre de composantes = %1").arg(counter-1);
		resultBillon.setMinValue(0);
		resultBillon.setMaxValue(counter-1);
	}

	Slice * extractConnexComponents( const Slice & slice, const int & minimumSize, const int & threshold )
	{
		Slice *bigestComponentsInSlice = new Slice(slice.n_cols,slice.n_rows);
		extractConnexComponents( *bigestComponentsInSlice, slice, minimumSize, threshold );
		return bigestComponentsInSlice;
	}

	void extractConnexComponents( Slice &resultSlice, const Slice & slice, const int & minimumSize, const int & threshold )
	{
		const uint width = slice.n_cols;
		const uint height = slice.n_rows;

		QMap<int, QList<iCoord2D> > connexComponentList;
		QMap<int, int> tableEquiv;
		QList<int> voisinage;
		uint j, i;
		int mini, nbLabel, label, currentEquiv;

		Slice labels(height, width);
		labels.fill(0);
		nbLabel = 0;
		//On parcourt une première fois la tranche
		for ( j=1 ; j<height ; ++j)
		{
			for ( i=1 ; i<width-1 ; ++i)
			{
				//Si on a un voxel
				if ( slice(j,i) > threshold )
				{
					//On sauvegarde la valeur des voisins non nuls
					voisinage.clear();
					//Voisinage de la face courante
					if (labels(j,i-1)) voisinage.append(labels(j,i-1));
					if (labels(j-1,i-1)) voisinage.append(labels(j-1,i-1));
					if (labels(j-1,i)) voisinage.append(labels(j-1,i));
					if (labels(j-1,i+1)) voisinage.append(labels(j-1,i+1));
					//Si ses voisins n'ont pas d'étiquette
					if ( voisinage.isEmpty() )
					{
						++nbLabel;
						labels(j,i) = nbLabel;
					}
					//Si ses voisins ont une étiquette
					else if ( voisinage.size() == 1 )
					{
						labels(j,i) = voisinage[0];
					}
					else
					{
						QList<int>::ConstIterator iterVoisin = voisinage.constBegin();
						mini = (*iterVoisin++);
						while ( iterVoisin != voisinage.constEnd() )
						{
							mini = qMin(mini,(*iterVoisin++));
						}
						labels(j,i) = mini;
						for ( iterVoisin = voisinage.constBegin() ; iterVoisin != voisinage.constEnd() ; ++iterVoisin )
						{
							if ( (*iterVoisin) > mini )
							{
								if ( tableEquiv.contains(*iterVoisin) )
								{
									currentEquiv = tableEquiv[*iterVoisin];
									if ( mini > currentEquiv )
									{
										tableEquiv[*iterVoisin] = mini;
										while (tableEquiv.contains(mini))
										{
											mini = tableEquiv[mini];
										}
										if ( currentEquiv < mini )
										{
											tableEquiv[mini] = currentEquiv;
											labels(j,i) = currentEquiv;
										}
										else if ( currentEquiv > mini )
										{
											tableEquiv[currentEquiv] = mini;
											labels(j,i) = mini;
										}
									}
									else if ( mini < currentEquiv )
									{
										tableEquiv[*iterVoisin] = currentEquiv;
										while (tableEquiv.contains(currentEquiv))
										{
											currentEquiv = tableEquiv[currentEquiv];
										}
										if ( currentEquiv > mini )
										{
											tableEquiv[currentEquiv] = mini;
											labels(j,i) = mini;
										}
										else if ( currentEquiv < mini )
										{
											tableEquiv[mini] = currentEquiv;
											labels(j,i) = currentEquiv;
										}
									}
								}
								else
								{
									tableEquiv[*iterVoisin] = mini;
								}
							}
						}
					}
				}
			}
		}

		//Résolution des chaines dans la table d'équivalence
		QMap<int, int>::ConstIterator iterTable;
		int value;
		for ( iterTable = tableEquiv.constBegin() ; iterTable != tableEquiv.constEnd() ; ++iterTable )
		{
			value = iterTable.value();
			while (tableEquiv.contains(value))
			{
				value = tableEquiv[value];
			}
			tableEquiv[iterTable.key()] = value;
		}
		for ( j=0 ; j<height ; ++j )
		{
			for ( i=0 ; i<width ; ++i )
			{
				label = labels(j,i);
				//Si on a un voxel
				if (label)
				{
					if (tableEquiv.contains(label))
					{
						labels(j,i) = tableEquiv[label];
						label = labels(j,i);
					}
					if (!connexComponentList.contains(label)) connexComponentList[label] = QList<iCoord2D>();
					connexComponentList[label].append(iCoord2D(i,j));
				}
			}
		}

		QMap<int, QList<iCoord2D> >::ConstIterator iterComponents;
		QList<iCoord2D>::ConstIterator iterCoords;
		int counter = 1;
		resultSlice.fill(0);
		for ( iterComponents = connexComponentList.constBegin() ; iterComponents != connexComponentList.constEnd() ; ++iterComponents )
		{
			if ( iterComponents.value().size() > minimumSize )
			{
				for ( iterCoords = iterComponents.value().constBegin() ; iterCoords != iterComponents.value().constEnd() ; ++ iterCoords )
				{
					resultSlice((*iterCoords).y,(*iterCoords).x) = counter;
				}
				++counter;
			}
		}
	}

	namespace
	{
		int twoPassAlgorithm( const Slice &oldSlice, const Slice &currentSlice, Slice &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold )
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
			for ( j=1 ; j<height-1 ; ++j)
			{
				for ( i=1 ; i<width-1 ; ++i)
				{
					//Si on a un voxel
					if ( currentSlice(j,i) > threshold )
					{
						//On sauvegarde la valeur des voisins non nuls
						voisinage.clear();
						//Voisinage de la face courante
						if (labels(j,i-1)) voisinage.append(labels(j,i-1));
						if (labels(j-1,i-1))voisinage.append(labels(j-1,i-1));
						if (labels(j-1,i)) voisinage.append(labels(j-1,i));
						if (labels(j-1,i+1)) voisinage.append(labels(j-1,i+1));
						oldStart = voisinage.size();
						//Voisinage de la face arrière
						if (oldSlice(j-1,i-1)) voisinage.append(oldSlice(j-1,i-1));
						if (oldSlice(j-1,i)) voisinage.append(oldSlice(j-1,i));
						if (oldSlice(j-1,i+1)) voisinage.append(oldSlice(j-1,i+1));
						if (oldSlice(j,i-1)) voisinage.append(oldSlice(j,i-1));
						if (oldSlice(j,i)) voisinage.append(oldSlice(j,i));
						if (oldSlice(j,i+1)) voisinage.append(oldSlice(j,i+1));
						if (oldSlice(j+1,i-1)) voisinage.append(oldSlice(j+1,i-1));
						if (oldSlice(j+1,i)) voisinage.append(oldSlice(j+1,i));
						if (oldSlice(j+1,i+1)) voisinage.append(oldSlice(j+1,i+1));
						//Si ses voisins n'ont pas d'étiquette
						if ( voisinage.isEmpty() )
						{
							++nbLabel;
							labels(j,i) = nbLabel;
						}
						//Si ses voisins ont une étiquette
						else
						{
							QList<int>::ConstIterator iterVoisin = voisinage.constBegin();
							mini = (*iterVoisin++);
							while ( iterVoisin != voisinage.constEnd() )
							{
								mini = qMin(mini,(*iterVoisin++));
							}
							//Attribution de la valeur minimale au voxel courant
							labels(j,i) = mini;
							isOld = connexComponentList.contains(mini);
							//Mise à jour de la table d'équivalence pour la face courante
							//et fusion des liste de sommets si un voxel fusionne des composantes connexes antérieures
							for ( int ind=0 ; ind<oldStart ; ++ind )
							{
								if ( voisinage[ind] > mini )
								{
									tableEquiv[voisinage[ind]] = mini;
									if (isOld && connexComponentList.contains(voisinage[ind]))
									{
										connexComponentList[mini].append(connexComponentList.take(voisinage[ind]));
									}
								}
							}
							if ( isOld )
							{
								for ( int ind=oldStart ; ind<voisinage.size() ; ++ind )
								{
									if ( voisinage[ind] != mini )
									{
										if ( mini>voisinage[ind] )
										{
											sup = mini;
											inf = voisinage[ind];
										}
										else
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
			QMap<int, int>::ConstIterator iterTable;
			int value;
			for ( iterTable = tableEquiv.constBegin() ; iterTable != tableEquiv.constEnd() ; ++iterTable )
			{
				value = iterTable.value();
				while (tableEquiv.contains(value))
				{
					value = tableEquiv[value];
				}
				tableEquiv[iterTable.key()] = value;
			}
			for ( j=0 ; j<height ; ++j )
			{
				for ( i=0 ; i<width ; ++i )
				{
					label = labels(j,i);
					//Si on a un voxel
					if (label)
					{
						if (tableEquiv.contains(label))
						{
							labels(j,i) = tableEquiv[label];
							label = labels(j,i);
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
