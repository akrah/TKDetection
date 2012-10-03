#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

#include <armadillo>
template <typename T> class QVector;
class QVector2D;
class QVector3D;

namespace OpticalFlow
{
	typedef QVector< QVector< QVector3D > > PartialDerivatives;
	typedef QVector< QVector< QVector2D > > VectorsField;

	VectorsField * compute( const arma::icube &cube, const int &k, const qreal &alpha = FLOW_ALPHA_DEFAULT, const qreal &epsilon = FLOW_EPSILON_DEFAULT, const int &maxIter = FLOW_MAXIMUM_ITERATIONS );
}

#endif // OPTICALFLOW_H
