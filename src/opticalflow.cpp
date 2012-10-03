#include "inc/opticalflow.h"

#include <inc/billon_def.h>

#include <qmath.h>
#include <QDebug>

namespace OpticalFlow {

	namespace {
		void computePartialDerivatives( const icube &cube, const int &k, PartialDerivatives &E );
		void computeLaplacian( const VectorsField &flow, VectorsField &laplacian );
	}

	// Donne le champ de vecteurs associé (sauf la dernière ligne et la dernière colonne)
	VectorsField * compute( const icube &cube, const int &k, const qreal &alpha, const qreal &epsilon, const int &maxIter )
	{
		VectorsField * flow = 0;
		if ( k>=0 && k<static_cast<int>(cube.n_slices)-1 ) {
			const int &width = cube.slice(k).n_cols-1;
			const int &height = cube.slice(k).n_rows-1;
			if ( width>2 && height>2 ) {
				flow = new VectorsField( height+1, QVector<QVector2D>( width+1 ) );

				PartialDerivatives E( height, QVector<QVector3D>( width )  );
				computePartialDerivatives( cube, k, E );

				VectorsField laplacian( height+1, QVector<QVector2D>( width+1 ) );
				int n = 0;
				qreal stoppingCriterion = 1.;

				qreal temp, Ex, Ey, Et;
				QVector2D oldFlow;
				while ( n++<maxIter && stoppingCriterion>epsilon ) {
					qDebug() << "Iteration" << n+1 << "/" << maxIter;
					qDebug() << "    stoppingCriterion = " << stoppingCriterion << " > " << epsilon ;
					computeLaplacian(*flow,laplacian);
					stoppingCriterion = 0.;
					for ( int j=0 ; j<height ; j++ ) {
						for ( int i=0 ; i<width ; i++ ) {
							const QVector2D &lapIJ = laplacian[j][i];
							QVector2D &newFlow = (*flow)[j][i];
							oldFlow = QVector2D((*flow)[j][i]);
							Ex = E[j][i].x();
							Ey = E[j][i].y();
							Et = E[j][i].z();
							temp = (Ex*lapIJ.x() + Ey*lapIJ.y() + Et) / (qPow(alpha,2) + qPow(Ex,2) + qPow(Ey,2));
							newFlow.setX(lapIJ.x() - Ex*temp);
							newFlow.setY(lapIJ.y() - Ey*temp);
							stoppingCriterion += qPow(newFlow.x() - oldFlow.x(),2) + qPow(newFlow.y() - oldFlow.y(),2);
						}
					}
					stoppingCriterion /= height*width;
				}
			}
		}
		return flow;
	}

	namespace {
		// Donne les dérivées partielles de la tranche K saur pour la dernière ligne et la dernière colonne
		inline
		void computePartialDerivatives( const icube &cube, const int &k, PartialDerivatives &E ) {
			const Slice &sliceK = cube.slice(k);
			const Slice &sliceK1 = cube.slice(k+1);
			const int width = sliceK.n_cols-1;
			const int height = sliceK.n_rows-1;
			qreal Eijk, Eijk1, Eij1k, Eij1k1, Ei1jk, Ei1jk1, Ei1j1k, Ei1j1k1;
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

					E[j][i].setX( (Eij1k - Eijk + Ei1j1k - Ei1jk + Eij1k1 - Eijk1 + Ei1j1k1 - Ei1jk1)/4. );
					E[j][i].setY( (Ei1jk - Eijk + Ei1j1k - Eij1k + Ei1jk1 - Eijk1 + Ei1j1k1 - Eij1k1)/4. );
					E[j][i].setZ( (Eijk1 - Eijk + Ei1jk1 - Ei1jk + Eij1k1 - Eij1k + Ei1j1k1 - Ei1j1k)/4. );
				}
			}
		}

		// Donne le Laplacien de field
		inline
		void computeLaplacian( const VectorsField &field, VectorsField &res ) {
			const int &width = field.size() - 1;
			const int &height = field[0].size() - 1;
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
	}
}
