#include "inc/lowess.h"

#include "inc/interval.h"

#include <QVector>
#include <qmath.h>

#include <gsl/gsl_multifit.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_blas.h>

#define ARMA_DONT_USE_ATLAS
#include <armadillo>

Lowess::Lowess( const qreal &bandwidth  ) : _bandWidth(bandwidth)
{
}

const qreal &Lowess::bandWidth() const
{
	return _bandWidth;
}

void Lowess::setBandWidth( const qreal &bandwidth )
{
	_bandWidth = bandwidth;
}

void Lowess::compute( const QVector<qreal> &datas, QVector<qreal> &interpolatedDatas, QVector<qreal> &residus ) const
{
	// Create test dataset.
	const int &size = datas.size();
	interpolatedDatas.resize(size);
	residus.resize(size);

	if ( !size ) return;

	// Loess neighborhood parameter.
	// XXX: Careful, alpha is assume to be <= 1 here.
	const uint q = qFloor(size * _bandWidth);

	// Working arrays. Yes we need both distances and sortedDistances.
	gsl_matrix *X = gsl_matrix_alloc(size, 3);
	gsl_vector *weights = gsl_vector_alloc(size);
	gsl_vector *c = gsl_vector_alloc(3);
	gsl_vector *x = gsl_vector_alloc(3);

	gsl_vector_const_view yValues = gsl_vector_const_view_array(datas.data(), size);

	arma::Col<qreal> distances(size);
	arma::Col<qreal> sortedDistances(size);

	qreal y, yi, wi, alpha0, alpha;
	int i, j;

	// Setup the model matrix X for a quadratic fit.
	for ( i=0 ; i<size ; ++i )
	{
		gsl_matrix_set(X, i, 0, 1.0);
		gsl_matrix_set(X, i, 1, i);
		gsl_matrix_set(X, i, 2, qPow(i,2));
	}

	gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(size, 3);
	gsl_matrix *A = work->A;
	gsl_matrix *Q = work->Q;
	gsl_matrix *QSI = work->QSI;
	gsl_vector *S = work->S;
	gsl_vector *t = work->t;
	gsl_vector *xt = work->xt;
	gsl_vector *D = work->D;

	gsl_vector_view row, column;

	for ( i=0 ; i<size ; ++i )
	{
		// TODO : Ici on sait directement la distance en foction de i et j car on n'est pas sur un nuage de points.
		// Il faudrait donc optimiser le calcul des poids.
		// Et peut-être carrément remonter la condition des fonction tricube et epanechnikov ici.

		// Compute distances.
		for ( j=0 ; j<size ; ++j )
		{
			distances[j] = qAbs( j-i );
		}

		// Sort distances in order from smallest to largest.
		sortedDistances = arma::sort(distances);

		// Compute weights.
		for ( j=0 ; j<size ; ++j )
		{
			//gsl_vector_set(weights, j, tricubeKernel(distances[j], sortedDistances[q]));
			gsl_vector_set(weights, j, epanechnikovKernel(distances[j], sortedDistances[q]));
		}

/******************************/

		/* Scale X, A = sqrt(w) X */
		gsl_matrix_memcpy (A, X);
		for ( j=0 ; j<size ; ++j )
		{
			wi = gsl_vector_get (weights, j);
			if (wi < 0)	wi = 0;
			{
				row = gsl_matrix_row (A, j);
				gsl_vector_scale (&row.vector, sqrt (wi));
			}
		}

		/* Balance the columns of the matrix A if requested */
		gsl_linalg_balance_columns (A, D);

		/* Decompose A into U S Q^T */
		gsl_linalg_SV_decomp_mod (A, QSI, Q, S, xt);

		/* Solve sqrt(w) y = A c for c, by first computing t = sqrt(w) y */
		for ( j=0 ; j<size ; ++j )
		{
			wi = gsl_vector_get (weights, j);
			yi = gsl_vector_get (&(yValues.vector), j);
			if (wi < 0)	wi = 0;
			gsl_vector_set (t, j, sqrt (wi) * yi);
		}
		gsl_blas_dgemv (CblasTrans, 1.0, A, t, 0.0, xt);

		/* Scale the matrix Q, Q' = Q S^-1 */
		gsl_matrix_memcpy (QSI, Q);
		alpha0 = gsl_vector_get (S, 0);
		for ( j=0 ; j<3 ; ++j )
		{
			column = gsl_matrix_column (QSI, j);
			alpha = gsl_vector_get (S, j);

			if (alpha <= 2.2204460492503131e-16 * alpha0) alpha = 0.0;
			else alpha = 1.0 / alpha;
			gsl_vector_scale (&column.vector, alpha);
		}

		/* Solution */
		gsl_blas_dgemv (CblasNoTrans, 1.0, QSI, xt, 0.0, c);

		/* Unscale the balancing factors */
		gsl_vector_div (c, D);

/*****************************/

		gsl_vector_set(x, 0, 1.0);
		gsl_vector_set(x, 1, i);
		gsl_vector_set(x, 2, qPow(i,2));
		gsl_blas_ddot(x, c, &y);

		interpolatedDatas[i] = y;
		residus[i] = datas[i]-y;
	}



	gsl_multifit_linear_free(work);
	gsl_matrix_free(X);
	gsl_vector_free(weights);
	gsl_vector_free(c);
	gsl_vector_free(x);
}

qreal Lowess::tricubeKernel( const qreal &u, const qreal &t ) const
{
	qreal res = 0.0;
	// 0 <= u < t
	//if ( (qFuzzyIsNull(u) || u > 0.0) && (u < t) )
	if ( u <= t )
		// (1 - (u/t)^3)^3
		res = qPow( ( 1.0 - qPow(u/t, 3.0) ), 3.0 );
	return res;
}

qreal Lowess::epanechnikovKernel( const qreal &u, const qreal &t ) const
{
	qreal res = 0.0;
	// 0 <= u < t
	//if ( !qFuzzyIsNull(t) && (t-u) >= 0 )
	if ( u <= t )
		// (1 - (u/t)^3)^3
		res = ( 1.0 - qPow(u/t, 2.0) );
	return res;
}
