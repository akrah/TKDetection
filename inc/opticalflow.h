#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

#include "opticalflow_def.h"

#include <armadillo>
using namespace arma;

namespace OpticalFlow
{
	VectorsField * compute( const icube &cube, const int &k, const qreal &alpha = FLOW_ALPHA_DEFAULT, const qreal &epsilon = FLOW_EPSILON_DEFAULT, const int &maxIter = FLOW_MAXIMUM_ITERATIONS );
}

#endif // OPTICALFLOW_H
