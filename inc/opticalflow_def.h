#ifndef OPTICALFLOW_DEF_H
#define OPTICALFLOW_DEF_H

#include <QVector>
#include <QVector2D>
#include <QVector3D>

#define FLOW_ALPHA_DEFAULT 7
#define FLOW_EPSILON_DEFAULT 0.001f
#define FLOW_MAXIMUM_ITERATIONS 100

typedef QVector< QVector< QVector3D > > PartialDerivatives;
typedef QVector< QVector< QVector2D > > VectorsField;

#endif // OPTICALFLOW_DEF_H
