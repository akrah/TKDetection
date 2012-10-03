#ifndef DEF_OPTICALFLOW_H
#define DEF_OPTICALFLOW_H

template <typename T> class QVector;
class QVector3D;
class QVector2D;
typedef QVector< QVector< QVector3D > > PartialDerivatives;
typedef QVector< QVector< QVector2D > > VectorsField;

#endif // DEF_OPTICALFLOW_H
