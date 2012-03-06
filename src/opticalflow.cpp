#include "inc/opticalflow.h"

#include <QDebug>
#include <qmath.h>

OpticalFlow::OpticalFlow( const icube *cube ) : _cube(cube), _alpha(ALPHA_DEFAULT), _epsilon(EPSILON_DEFAULT), _maxIter(MAXIMUM_ITERATIONS) {
}

// Donne les dérivées partielles de la tranche K saur pour la dernière ligne et la dernière colonne
PartialDerivatives * OpticalFlow::getPartialDerivatives( const int &k ) const {
	PartialDerivatives *E = 0;
	if ( _cube != 0 && k>=0 && k<static_cast<int>(_cube->n_slices)-1 ) {
		const imat &sliceK = _cube->slice(k);
		const imat &sliceK1 = _cube->slice(k+1);
		const int width = sliceK.n_cols-1;
		const int height = sliceK.n_rows-1;

		if ( width > 0 && height > 0 ) {
			E = new PartialDerivatives(width,height);

			float Eijk, Eijk1, Eij1k, Eij1k1, Ei1jk, Ei1jk1, Ei1j1k, Ei1j1k1;
			for ( int j=0 ; j<height ; j++ ) {
				for ( int i=0 ; i<width ; i++ ) {
					Eijk = sliceK.at(j,i);
					Eij1k = sliceK.at(j+1,i);
					Ei1jk = sliceK.at(j,i+1);
					Ei1j1k = sliceK.at(j+1,i+1);

					Eijk1 = sliceK1.at(j,i);
					Eij1k1 = sliceK1.at(j+1,i);
					Ei1jk1 = sliceK1.at(j,i+1);
					Ei1j1k1 = sliceK1.at(j+1,i+1);

					E->x(i,j) = (Eij1k - Eijk + Ei1j1k - Ei1jk + Eij1k1 - Eijk1 + Ei1j1k1 - Ei1jk1)/4.;
					E->y(i,j) = (Ei1jk - Eijk + Ei1j1k - Eij1k + Ei1jk1 - Eijk1 + Ei1j1k1 - Eij1k1)/4.;
					E->t(i,j) = (Eijk1 - Eijk + Ei1jk1 - Ei1jk + Eij1k1 - Eij1k + Ei1j1k1 - Ei1j1k)/4.;
				}
			}
		}
	}
	return E;
}

// Donne le Laplacien de field
VectorsField * OpticalFlow::computeLaplacian( const VectorsField &field ) const {
	VectorsField * res_p = 0;
	// Field doit contenir au moins 3x3 vecteurs
	if ( field.size() > 2 && field[0].size() > 2 ) {
		const int &width = field.size() - 1;
		const int &height = field[0].size() - 1;

		res_p = new VectorsField( height+1, QVector<QVector2D>( width+1, QVector2D(0.,0.) ) );
		VectorsField &res = *res_p;

		for ( int j=1 ; j<height ; j++ ) {
			// Calcul des Laplaciens qui ne sont pas sur les bords
			for ( int i=1 ; i<width ; i++ ) {
				res[j][i] =  ((field[j][i-1] + field[j][i+1] + field[j-1][i] + field[j+1][i])/6.f) +
						((field[j-1][i-1] + field[j-1][i+1] + field[j+1][i-1] + field[j+1][i+1])/12.f);
			}
			// Calcul des Laplaciens aux extrémités des lignes 1 à n_rows-1
			res[j][0] = res[j][1];
			res[j][height]  = res[j][height-1];
		}
		// Calcul des Laplaciens aux extrémités sur la première et la dernière ligne
		res[0][0] = res[1][1];
		res[0][width] = res[1][width-1];
		res[height][0] = res[height-1][1];
		res[height][width] = res[height-1][width-1];
		// Calcul des Laplaciens sur la première et la dernière ligne, sauf les extrémités
		for ( int i=1 ; i<width ; i++ ) {
			res[0][i] = res[1][i];
		}
	}
	return res_p;
}

// Donne le champ de vecteurs associé (sauf la dernière ligne et la dernière colonne)
VectorsField * OpticalFlow::computeFlowOnSlice( const int &k ) {
	VectorsField * flow = 0;
	if ( _cube != 0 && k>=0 && k<static_cast<int>(_cube->n_slices) ) {
		const imat &sliceK = _cube->slice(k);
		const int &width = sliceK.n_cols-1;
		const int &height = sliceK.n_rows-1;

		if ( width>2 && height>2 ) {
			int n = 0;
			float stoppingCriterion = 1.;
			VectorsField * laplacian_p = 0;
			PartialDerivatives * E = getPartialDerivatives(k);
			if ( E != 0 ) {
				flow = new VectorsField(sliceK.n_rows, QVector<QVector2D>(sliceK.n_cols, QVector2D(0.,0.)));

				while ( n++<_maxIter && stoppingCriterion>_epsilon ) {
					qDebug() << "Iteration" << n+1 << "/" << _maxIter;
					qDebug() << "    stoppingCriterion = " << stoppingCriterion << " > " << _epsilon ;
					laplacian_p = computeLaplacian(*flow);
					const VectorsField& laplacian = *laplacian_p;
					stoppingCriterion = 0.;
					for ( int j=0 ; j<height ; j++ ) {
						for ( int i=0 ; i<width ; i++ ) {
							const qreal temp = (E->x.at(j,i)*laplacian[j][i].x() + E->y.at(j,i)*laplacian[j][i].y() + E->t.at(j,i)) / (_alpha*_alpha + E->x.at(j,i)*E->x.at(j,i) + E->y.at(j,i)*E->y.at(j,i));
							const QVector2D Un = (*flow)[j][i];
							(*flow)[j][i].setX(laplacian[j][i].x() - E->x.at(j,i)*temp);
							(*flow)[j][i].setY(laplacian[j][i].y() - E->y.at(j,i)*temp);
							stoppingCriterion += qPow((*flow)[j][i].x() - Un.x(),2) + qPow((*flow)[j][i].y() - Un.y(),2);
						}
					}
					delete laplacian_p;
					laplacian_p = 0;
					stoppingCriterion /= height*width;
				}
				delete E;
			}
		}
	}
	return flow;
}
