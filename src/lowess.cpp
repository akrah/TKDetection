#include "inc/lowess.h"

#include <QVector>
#include <qmath.h>

#include <gsl/gsl_multifit.h>
#include <armadillo>

Lowess::Lowess( const qreal &bandwidth ) : _bandWidth(bandwidth)
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

void Lowess::compute( const QVector<qreal> &datas, QVector<qreal> &interpolatedDatas ) const
{
	// Create test dataset.
	const int size = datas.size();

	// Loess neighborhood parameter.
	// XXX: Careful, alpha is assume to be <= 1 here.
	const uint q = qFloor(size * _bandWidth);

	// Working arrays. Yes we need both distances and sortedDistances.
	gsl_matrix *X = gsl_matrix_alloc(size, 3);
	gsl_matrix *cov = gsl_matrix_alloc(3, 3);
	gsl_vector *weights = gsl_vector_alloc(size);
	gsl_vector *c = gsl_vector_alloc(3);
	gsl_vector *x = gsl_vector_alloc(3);

	gsl_vector_const_view yValues = gsl_vector_const_view_array(datas.data(), size);

	arma::Col<qreal> distances(size);
	arma::Col<qreal> sortedDistances(size);

	qreal y, yErr, chisq;
	int i, j;

	// Setup the model matrix X for a quadratic fit.
	for ( i=0 ; i<size ; ++i )
	{
		gsl_matrix_set(X, i, 0, 1.0);
		gsl_matrix_set(X, i, 1, i);
		gsl_matrix_set(X, i, 2, qPow(i,2));
	}

	interpolatedDatas.resize(size);

	for ( i=0 ; i<size ; ++i )
	{
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
			//weights.at(j) = tricube(distances[j], sortedDistances[q]);
			gsl_vector_set(weights, j, tricube(distances[j], sortedDistances[q]));
		}

		gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(size, 3);
		gsl_multifit_wlinear(X, weights, &(yValues.vector), c, cov, &chisq, work);
		gsl_multifit_linear_free(work);

		gsl_vector_set(x, 0, 1.0);
		gsl_vector_set(x, 1, i);
		gsl_vector_set(x, 2, qPow(i,2));
		gsl_multifit_linear_est (x, c, cov, &y, &yErr);

		interpolatedDatas[i] = y;
	}

	gsl_matrix_free(X);
	gsl_vector_free(weights);
	gsl_vector_free(c);
	gsl_matrix_free(cov);
}

qreal Lowess::tricube( const qreal &u, const qreal &t ) const
{
	// 0 <= u < t
	if ( (qFuzzyIsNull(u) || u > 0.0) && (u < t) )
	{
		// (1 - (u/t)^3)^3
		return qPow( ( 1.0 - qPow(u/t, 3.0) ), 3.0 );
	}
	else return 0.0;
}
