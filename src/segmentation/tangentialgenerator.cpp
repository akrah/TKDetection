#include "inc/segmentation/tangentialgenerator.h"

#include <qmath.h>

#include "inc/billon.h"
#include "inc/piechart.h"

TangentialGenerator::TangentialGenerator( const int &minIntensity, const bool &trilinearInterpolation ) :
	_minIntensity(minIntensity), _trilinearInterpolation(trilinearInterpolation),
	_currentSliceInterval(Interval<uint>(0,0)), _currentAngularInterval(Interval<uint>(0,0)),
	_origin(QVector3D(0,0,0)), _angularRange(0.), _bisectorOrientation(0.), _depth(0.), _width(0), _height(0), _lateralShift(0), _verticalShift(0),
	_quaterX(QQuaternion::fromAxisAndAngle( 1., 0., 0., -90.)), _quaterY(QQuaternion::fromAxisAndAngle( 0., 1., 0., 90.)),
	_quaterZ(QQuaternion::fromAxisAndAngle( 0., 0., 1., 0.)), _quaterRot( _quaterX*_quaterY*_quaterZ ),
	_zShiftStep(_quaterRot.rotatedVector(QVector3D( 0., 0., 1. )))
{
}

TangentialGenerator::TangentialGenerator( const TangentialGenerator &tangentialGenerator ) :
	_minIntensity(tangentialGenerator._minIntensity), _trilinearInterpolation(tangentialGenerator._trilinearInterpolation),
	_currentSliceInterval(tangentialGenerator._currentSliceInterval), _currentAngularInterval(tangentialGenerator._currentAngularInterval),
	_origin(tangentialGenerator._origin), _angularRange(tangentialGenerator._angularRange), _bisectorOrientation(tangentialGenerator._bisectorOrientation),
	_depth(tangentialGenerator._depth), _width(tangentialGenerator._width), _height(tangentialGenerator._height),
	_lateralShift(tangentialGenerator._lateralShift), _verticalShift(tangentialGenerator._verticalShift),
	_quaterX(tangentialGenerator._quaterX), _quaterY(tangentialGenerator._quaterY),
	_quaterZ(tangentialGenerator._quaterZ), _quaterRot(tangentialGenerator._quaterRot),
	_zShiftStep(tangentialGenerator._zShiftStep)
{
}

TangentialGenerator::~TangentialGenerator() {}

void TangentialGenerator::setSliceInterval( const Billon &billon, const Interval<uint> &sliceInterval )
{
	_currentSliceInterval = sliceInterval;
	_height = sliceInterval.count();
	_verticalShift = qFloor(_height/2.);
	const uint midSliceInterval = sliceInterval.mid();
	_origin = QVector3D(billon.pithCoord(midSliceInterval).x, billon.pithCoord(midSliceInterval).y, midSliceInterval);
}

void TangentialGenerator::setAngularInterval( const Billon &billon, const Interval<uint> &angularInterval, const PieChart &pieChart )
{
	_currentAngularInterval = angularInterval;
	_angularRange = _currentAngularInterval.count();
	_bisectorOrientation = (_currentAngularInterval.min()+_angularRange/2.)*pieChart.angleStep();
	_quaterZ = QQuaternion::fromAxisAndAngle( 0., 0., 1., _bisectorOrientation*RAD_TO_DEG_FACT );

	/* Recherche de l'écorce dans la direction de la bissectrice du secteur angulaire sur la coupe au milieu de l'intervalle de coupes */
	const rVec2D direction(qCos(_bisectorOrientation),qSin(_bisectorOrientation));
	const rCoord2D originPith( _origin.x(), _origin.y() );
	rCoord2D edge = originPith + direction*30;
	while ( edge.x>0 && edge.y>0 && edge.x<billon.n_cols && edge.y<billon.n_rows && billon.slice(_origin.z())(edge.y,edge.x) > _minIntensity )
	{
		edge += direction;
	}
	_depth = rVec2D(edge-originPith).norm();
	_width = qFloor(2 * qTan(_angularRange*pieChart.angleStep()/2.) * _depth);
	_lateralShift = qFloor(_width/2.);

	_quaterRot = _quaterZ * _quaterX * _quaterY;
	_zShiftStep = _quaterRot.rotatedVector( QVector3D( 0., 0., 1. ) );
}

void TangentialGenerator::updateIntervals( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &angularInterval, const PieChart &pieChart )
{
	setSliceInterval(billon,sliceInterval);
	setAngularInterval(billon,angularInterval,pieChart);
}

void TangentialGenerator::setKnotArea( const Billon &billon, const QRect &knotArea, const PieChart &pieChart )
{
	setSliceInterval( billon, Interval<uint>(knotArea.left(),knotArea.right()) );
	setAngularInterval( billon, Interval<uint>(knotArea.top(),knotArea.bottom()), pieChart );
}

void TangentialGenerator::setMinIntensity( const int &minIntensity )
{
	_minIntensity = minIntensity;
}

void TangentialGenerator::enableTrilinearInterpolation( bool enable )
{
	_trilinearInterpolation = enable;
}

const int &TangentialGenerator::minIntensity() const
{
	return _minIntensity;
}

const bool &TangentialGenerator::trilinearInterpolation() const
{
	return _trilinearInterpolation;
}

const Interval<uint> &TangentialGenerator::currentSliceInterval() const
{
	return _currentSliceInterval;
}
const Interval<uint> &TangentialGenerator::currentAngularInterval() const
{
	return _currentAngularInterval;
}

const QVector3D &TangentialGenerator::origin() const
{
	return _origin;
}

QVector3D TangentialGenerator::originRelativeToSliceInterval() const
{
	return QVector3D( _origin.x(), _origin.y(), _origin.z()-_currentSliceInterval.min() );
}

const qreal &TangentialGenerator::angularRange() const
{
	return _angularRange;
}

const qreal &TangentialGenerator::bisectorOrientation() const
{
	return _bisectorOrientation;
}

const qreal &TangentialGenerator::depth() const
{
	return _depth;
}

const uint &TangentialGenerator::width() const
{
	return _width;
}

const uint &TangentialGenerator::height() const
{
	return _height;
}

const int &TangentialGenerator::lateralShift() const
{
	return _lateralShift;
}

const int &TangentialGenerator::vertcalShift() const
{
	return _verticalShift;
}

const QQuaternion &TangentialGenerator::quaterX() const
{
	return _quaterX;
}

const QQuaternion &TangentialGenerator::quaterY() const
{
	return _quaterY;
}

const QQuaternion &TangentialGenerator::quaterZ() const
{
	return _quaterZ;
}

const QQuaternion &TangentialGenerator::quaterRot() const
{
	return _quaterRot;
}

const QVector3D &TangentialGenerator::zShiftStep() const
{
	return _zShiftStep;
}

QVector3D TangentialGenerator::zShiftStep( const qreal &stepInZ ) const
{
	return _quaterRot.rotatedVector( QVector3D( 0., 0., stepInZ ) );
}

QVector3D TangentialGenerator::rotate( const iCoord3D &initialCoord ) const
{
	return _quaterRot.rotatedVector( QVector3D( initialCoord.x-_lateralShift, initialCoord.y-_verticalShift, 0 ) ) + (_origin + _zShiftStep*initialCoord.z);
}

QVector3D TangentialGenerator::rotate( const QVector3D &initialCoord ) const
{
	return _quaterRot.rotatedVector( QVector3D( initialCoord.x()-_lateralShift, initialCoord.y()-_verticalShift, 0 ) ) + (_origin + _zShiftStep*initialCoord.z());
}

Billon* TangentialGenerator::execute( const Billon &billon )
{
	/* Hauteur et largeur des coupes transversales */
	const int billonWidthMinusOne = billon.n_cols-1;
	const int billonHeightMinusOne = billon.n_rows-1;
	const int billonDepthMinusOne = billon.n_slices-1;

	const uint nbSlices = qCeil(_depth);

	// Inversion width et height pour correspondre à la rotation de 90°
//	const qreal cosBisector = qCos(_bisectorOrientation);
//	const qreal sinBisector = qSin(_bisectorOrientation);
	Billon * tangentialBillon = new Billon(_width,_height,nbSlices);
//	tangentialBillon->setVoxelSize( qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)),
//									billon.voxelDepth(),
//									(billon.voxelWidth()*billon.voxelHeight())/qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)) );
	tangentialBillon->setVoxelSize( billon.voxelWidth(),
									billon.voxelDepth(),
									billon.voxelHeight() );
	tangentialBillon->setValueInterval( billon.valueInterval() );
	tangentialBillon->fill(_minIntensity);

	QVector3D origin(_origin);
	QVector3D initial, destination;

	const qreal semiKnotAreaWidthCoeff = _lateralShift / static_cast<qreal>( nbSlices );
	int j, i, iStart, iEnd;

	int x0,y0,z0;
	qreal x0Dist, y0Dist, z0Dist;
	qreal xFrontTop, xFrontBottom, xBackTop, xBackBottom, yFront, yBack;

	iStart = iEnd = 0;
	initial.setZ(0.);
	for ( uint k=0 ; k<nbSlices ; ++k )
	{
		Slice &slice = tangentialBillon->slice(k);
		iEnd = qMin(qRound(semiKnotAreaWidthCoeff*k),_lateralShift);
		iStart = -iEnd;
		for ( i=iStart ; i<iEnd ; ++i )
		{
			initial.setX(i);
			for ( j=-_verticalShift ; j<_verticalShift ; ++j )
			{
				initial.setY(j);
				destination = _quaterRot.rotatedVector(initial) + origin;
				x0 = qFloor(destination.x());
				y0 = qFloor(destination.y());
				z0 = qFloor(destination.z());
				if ( x0>0 && x0<billonWidthMinusOne &&
					 y0>0 && y0<billonHeightMinusOne &&
					 z0>0 && z0<billonDepthMinusOne )
				{
					if ( _trilinearInterpolation )
					{
						x0Dist = destination.x()-x0;
						y0Dist = destination.y()-y0;
						z0Dist = destination.z()-z0;
						xFrontTop = (1.-x0Dist)*billon(y0,x0,z0) + x0Dist*billon(y0,x0+1,z0);
						xFrontBottom = (1.-x0Dist)*billon(y0+1,x0,z0) + x0Dist*billon(y0+1,x0+1,z0);
						xBackTop = (1.-x0Dist)*billon(y0,x0,z0+1) + x0Dist*billon(y0,x0+1,z0+1);
						xBackBottom = (1.-x0Dist)*billon(y0+1,x0,z0+1) + x0Dist*billon(y0+1,x0+1,z0+1);
						yFront = (1.-y0Dist)*xFrontTop + y0Dist*xFrontBottom;
						yBack = (1.-y0Dist)*xBackTop + y0Dist*xBackBottom;
						// Rotation de 90° dans le sens horaire pour correspondre à l'orientation de l'article
						slice(j+_verticalShift,i+_lateralShift) = (1.-z0Dist)*yFront + z0Dist*yBack;
					}
					else
					{
						// Rotation de 90° dans le sens horaire pour correspondre à l'orientation de l'article
						slice(j+_verticalShift,i+_lateralShift) =	billon(y0,x0,z0);
					}
				}
			}
		}
		origin += _zShiftStep;
	}

	return tangentialBillon;
}
