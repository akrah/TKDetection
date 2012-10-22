#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

#include "define.h"
#include "def/def_billon.h"
#include "def/def_opticalflow.h"

#include <QtGlobal>

#include <armadillo>

template <typename T> class QVector;
class QVector2D;
class QVector3D;

namespace OpticalFlow
{
	VectorsField * compute( const Billon &billon, const int &k, const qreal &alpha = FLOW_ALPHA_DEFAULT, const qreal &epsilon = FLOW_EPSILON_DEFAULT, const int &maxIter = FLOW_MAXIMUM_ITERATIONS );
}

#endif // OPTICALFLOW_H
