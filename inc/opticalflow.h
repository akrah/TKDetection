#ifndef OPTICALFLOW_H
#define OPTICALFLOW_H

#include <QVector>
#include <QVector2D>

#include <armadillo>
using namespace arma;

#define ALPHA_DEFAULT 7
#define EPSILON_DEFAULT 0.001f
#define MAXIMUM_ITERATIONS 100

typedef struct PartialDerivatives_struct {
	fmat x;
	fmat y;
	fmat t;
	PartialDerivatives_struct( const int &width, const int &height ) : x(height,width), y(height,width), t(height,width) {}
} PartialDerivatives;

typedef QVector< QVector< QVector2D > > VectorsField;

class OpticalFlow
{
public:
	explicit OpticalFlow( const icube *cube );

	VectorsField * computeFlowOnSlice( const int &k );

private:
	PartialDerivatives * getPartialDerivatives( const int &k ) const;
	VectorsField * computeLaplacian( const VectorsField &flow ) const;

private:
	const icube * _cube;
	float _alpha;
	float _epsilon;
	int _maxIter;
};

#endif // OPTICALFLOW_H
