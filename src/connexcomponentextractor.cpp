#include "inc/connexcomponentextractor.h"

#include "inc/marrow_def.h"
#include "inc/billon.h"

#include <armadillo>

#include <QList>
#include <QPair>

#include <QDebug>

namespace ConnexComponentExtractor {

	namespace {
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
		int twoPassAlgorithm( arma::imat &oldSlice, arma::Slice &currentSlice, arma::imat &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold );
	}

	Billon * extractConnexComponent( Billon &billon, const int &minimumSize, const __billon_type__ &threshold ) {

		const uint width = billon.n_cols;
		const uint height = billon.n_rows;
		const uint depth = billon.n_slices;

		int nbLabel = 0;
		QMap<int, QList<iCoord3D> > connexComponentList;
		arma::imat* labels = new arma::imat(height, width);
		arma::imat* oldSlice = new arma::imat(height, width);
		oldSlice->fill(0);
		arma::imat* tmp;

		//On parcours les tranches 1 par 1
		for ( unsigned int k=0 ; k<depth ; k++ ) {
			nbLabel = twoPassAlgorithm((*oldSlice),billon.slice(k),(*labels),connexComponentList,k,nbLabel,threshold);
			tmp = oldSlice;
			oldSlice = labels;
			labels = tmp;
			tmp = 0;
		}

		delete labels;
		delete oldSlice;

		//On identifie les valeurs minimales en x, y, et z pour chaque composante
//		QMap<int, QPair<iCoord3D, iCoord3D> > saveMinMax;
//		int curVal;
//		iCoord3D currentCoord;

//		QMapIterator<int, QList<iCoord3D> > iter(connexComponentList);
//		while (iter.hasNext()) {
//			iter.next();
//			if ( iter.value().size() > minimumSize) {
//				curVal = iter.key();
//				saveMinMax[curVal] = QPair<iCoord3D, iCoord3D>(iter.value().first(), iter.value().first());
//				QListIterator<iCoord3D> coords(iter.value());
//				while (coords.hasNext()) {
//					currentCoord = coords.next();
//					saveMinMax[curVal].first.x = qMin(saveMinMax[curVal].first.x,currentCoord.x);
//					saveMinMax[curVal].first.y = qMin(saveMinMax[curVal].first.y,currentCoord.y);
//					saveMinMax[curVal].first.z = qMin(saveMinMax[curVal].first.z,currentCoord.z);

//					saveMinMax[curVal].second.x = qMax(saveMinMax[curVal].second.x,currentCoord.x);
//					saveMinMax[curVal].second.y = qMax(saveMinMax[curVal].second.y,currentCoord.y);
//					saveMinMax[curVal].second.z = qMax(saveMinMax[curVal].second.z,currentCoord.z);
//				}
//			}
//		}

		//On ajoute les composantes connexes à l'image
//		QMapIterator<int, QPair<iCoord3D, iCoord3D> > iterMinMax(saveMinMax);
//		iCoord3D pmin,pmax, unit;
//		unit = iCoord3D(1,1,1);
//		int counter = 1;
//		while (iterMinMax.hasNext()) {
//			iterMinMax.next();
//			pmin = iterMinMax.value().first - unit;
//			pmax = iterMinMax.value().second + unit;
//			Billon* con = new Billon(pmin, pmax);
//			con->setMaximumValue(1.0f);
//			QListIterator<iCoord3D> coords(connexComponentList[iterMinMax.key()]);
//			while (coords.hasNext()) {
//				currentCoord = coords.next();
//				con->value(currentCoord-pmin) = 1.0f;
//			}
//			if ( components.contains(counter) ) {
//				delete components[counter];
//				components.remove(counter);
//			}
//			components.insert(counter++,con);
//		}

		Billon* components = new Billon(width,height,depth);
		components->setVoxelSize(billon.voxelWidth(),billon.voxelHeight(),billon.voxelDepth());
		components->fill(0);

		QMapIterator<int, QList<iCoord3D> > iter(connexComponentList);
		iCoord3D currentCoord;
		int counter = 1;
		while (iter.hasNext()) {
			iter.next();
			if ( iter.value().size() > minimumSize ) {
				QListIterator<iCoord3D> coords(iter.value());
				while (coords.hasNext()) {
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

	namespace {
		int twoPassAlgorithm( arma::imat &oldSlice, arma::Slice &currentSlice, arma::imat &labels, QMap<int, QList<iCoord3D> > &connexComponentList, int k, int nbLabel, const __billon_type__ &threshold ) {
			const uint width = currentSlice.n_cols;
			const uint height = currentSlice.n_rows;

			QMap<int, int> tableEquiv;
			QList<int> voisinage;
			uint j, i;
			int mini, oldStart, sup, inf, label;
			bool isOld;
			labels.fill(0);
			//On parcourt une première fois la tranche
			for ( j=1 ; j<height-1 ; j++) {
				for ( i=1 ; i<width-1 ; i++) {
					//Si on a un voxel
					if ( currentSlice.at(j,i) > threshold ) {
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
						if ( voisinage.isEmpty() ) {
							nbLabel++;
							labels.at(j,i) = nbLabel;
						}
						//Si ses voisins ont une étiquette
						else {
							QListIterator<int> iterVoisin(voisinage);
							mini = iterVoisin.next();
							while(iterVoisin.hasNext()) {
								mini = qMin(mini,iterVoisin.next());
							}
							//Attribution de la valeur minimale au voxel courant
							labels.at(j,i) = mini;
							isOld = connexComponentList.contains(mini);
							//Mise à jour de la table d'équivalence pour la face courante
							//et fusion des liste de sommets si un voxel fusionne des composantes connexes antérieures
							for ( int ind=0 ; ind<oldStart ; ind++ ) {
								if ( voisinage[ind] > mini ) {
									tableEquiv[voisinage[ind]] = mini;
									if (isOld && connexComponentList.contains(voisinage[ind])) {
										connexComponentList[mini].append(connexComponentList.take(voisinage[ind]));
										//tableEquiv[voisinage[ind]] = mini; EST DEJA FAIT AU DESSUS DU IF
									}
								}
							}
							if ( isOld ) {
								for ( int ind=oldStart ; ind<voisinage.size() ; ind++ ) {
									if ( voisinage[ind] != mini ) {
										if ( mini>voisinage[ind] ) {
											sup = mini;
											inf = voisinage[ind];
										} else {
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
			while (iter.hasNext()) {
				iter.next();
				if (tableEquiv.contains(iter.value())) {
					tableEquiv[iter.key()] = tableEquiv[iter.value()];
				}
			}
			for ( j=0 ; j<height ; j++ ) {
				for ( i=0 ; i<width ; i++ ) {
					label = labels.at(j,i);
					//Si on a un voxel
					if (label) {
						if (tableEquiv.contains(label)) {
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
