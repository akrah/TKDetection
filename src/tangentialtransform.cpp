#include "inc/tangentialtransform.h"

#include <qmath.h>

#include "inc/billon.h"
#include "inc/piechart.h"

TangentialTransform::TangentialTransform( const int &minIntensity, const bool &trilinearInterpolation ) :
	_minIntensity(minIntensity), _trilinearInterpolation(trilinearInterpolation),
	_currentSliceInterval(Interval<uint>(0,0)), _currentAngularInterval(Interval<uint>(0,0)),
	_origin(QVector3D(0,0,0)), _angularRange(0.), _bisectorOrientation(0.), _depth(0.),
	_quaterX(QQuaternion::fromAxisAndAngle( 1., 0., 0., -90.)), _quaterY(QQuaternion::fromAxisAndAngle( 0., 1., 0., 90.)),
	_quaterZ(QQuaternion::fromAxisAndAngle( 0., 0., 1., 0.)), _quaterRot( _quaterX*_quaterY*_quaterZ ),
	_shiftStep(_quaterRot.rotatedVector(QVector3D( 0., 0., 1. )))
{
}

TangentialTransform::TangentialTransform( const TangentialTransform &tT ) :
	_minIntensity(tT._minIntensity), _trilinearInterpolation(tT._trilinearInterpolation),
	_currentSliceInterval(tT._currentSliceInterval), _currentAngularInterval(tT._currentAngularInterval),
	_origin(tT._origin), _angularRange(tT._angularRange), _bisectorOrientation(tT._bisectorOrientation), _depth(tT._depth),
	_quaterX(tT._quaterX), _quaterY(tT._quaterY),
	_quaterZ(tT._quaterZ), _quaterRot( tT._quaterRot ),
	_shiftStep( tT._shiftStep )
{
}

TangentialTransform::~TangentialTransform() {}

void TangentialTransform::setSliceInterval( const Billon &billon, const Interval<uint> &sliceInterval )
{
	_currentSliceInterval = sliceInterval;
	const uint midSliceInterval = sliceInterval.mid();
	_origin = QVector3D(billon.pithCoord(midSliceInterval).x, billon.pithCoord(midSliceInterval).y, midSliceInterval);
}

void TangentialTransform::setAngularInterval( const Billon &billon, const Interval<uint> &angularInterval )
{
	_currentAngularInterval = angularInterval;
	_angularRange = (_currentAngularInterval.max() + (_currentAngularInterval.isValid() ? 0. : PieChartSingleton::getInstance()->nbSectors())) - _currentAngularInterval.min() + 1;
	_bisectorOrientation = (_currentAngularInterval.min()+_angularRange/2.)*PieChartSingleton::getInstance()->angleStep();
	_quaterZ = QQuaternion::fromAxisAndAngle( 0., 0., 1., _bisectorOrientation*RAD_TO_DEG_FACT );

	/* Recherche de l'écorce dans la direction de la bissectrice du secteur angulaire sur la coupe au milieu de l'intervalle de coupes */
	const rVec2D direction(qCos(_bisectorOrientation),qSin(_bisectorOrientation));
	rCoord2D originPith( _origin.x(), _origin.y() );
	rCoord2D edge = originPith + direction*30;
	while ( edge.x>0 && edge.y>0 && edge.x<billon.n_cols && edge.y<billon.n_rows && billon.slice(_origin.z())(edge.y,edge.x) > _minIntensity )
	{
		edge += direction;
	}
	_depth = rVec2D(edge-originPith).norm();

	_quaterRot = _quaterZ * _quaterX * _quaterY;
	_shiftStep = _quaterRot.rotatedVector( QVector3D( 0., 0., 1. ) );
}

void TangentialTransform::updateIntervals( const Billon &billon, const Interval<uint> &sliceInterval, const Interval<uint> &angularInterval )
{
	setSliceInterval(billon,sliceInterval);
	setAngularInterval(billon,angularInterval);
}

void TangentialTransform::setMinIntensity( const int &minIntensity )
{
	_minIntensity = minIntensity;
}

void TangentialTransform::enableTrilinearInterpolation( bool enable )
{
	_trilinearInterpolation = enable;
}

const int &TangentialTransform::minIntensity() const
{
	return _minIntensity;
}

const bool &TangentialTransform::trilinearInterpolation() const
{
	return _trilinearInterpolation;
}

const Interval<uint> &TangentialTransform::currentSliceInterval() const
{
	return _currentSliceInterval;
}
const Interval<uint> &TangentialTransform::currentAngularInterval() const
{
	return _currentAngularInterval;
}

const QVector3D &TangentialTransform::origin() const
{
	return _origin;
}

QVector3D TangentialTransform::originRelativeToSliceInterval() const
{
	return QVector3D( _origin.x(), _origin.y(), _origin.z()-_currentSliceInterval.min() );
}

const qreal &TangentialTransform::angularRange() const
{
	return _angularRange;
}

const qreal &TangentialTransform::bisectorOrientation() const
{
	return _bisectorOrientation;
}

const qreal &TangentialTransform::depth() const
{
	return _depth;
}

const QQuaternion &TangentialTransform::quaterX() const
{
	return _quaterX;
}

const QQuaternion &TangentialTransform::quaterY() const
{
	return _quaterY;
}

const QQuaternion &TangentialTransform::quaterZ() const
{
	return _quaterZ;
}

const QQuaternion &TangentialTransform::quaterRot() const
{
	return _quaterRot;
}

const QVector3D &TangentialTransform::shiftStep() const
{
	return _shiftStep;
}

QVector3D TangentialTransform::shiftStep( const qreal &stepInZ ) const
{
	return _quaterRot.rotatedVector( QVector3D( 0., 0., stepInZ ) );
}

Billon* TangentialTransform::execute( const Billon &billon )
{
	/* Hauteur et largeur des coupes transversales */
	const int &billonWidthMinusOne = billon.n_cols-1;
	const int &billonHeightMinusOne = billon.n_rows-1;
	const int &billonDepthMinusOne = billon.n_slices-1;

	/* Hauteur et largeur des coupes tangentielles */
	const uint width = qFloor(2 * qTan(_angularRange*PieChartSingleton::getInstance()->angleStep()/2.) * _depth);
	const uint height = _currentSliceInterval.size()+1;
	const int widthOnTwo = qFloor(width/2.);
	const int heightOnTwo = qFloor(height/2.);

	const uint nbSlices = qCeil(_depth);

	// Inversion width et height pour correspondre à la rotation de 90°
//	const qreal cosBisector = qCos(_bisectorOrientation);
//	const qreal sinBisector = qSin(_bisectorOrientation);
	Billon * tangentialBillon = new Billon(width,height,nbSlices);
//	tangentialBillon->setVoxelSize( qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)),
//									billon.voxelDepth(),
//									(billon.voxelWidth()*billon.voxelHeight())/qSqrt(qPow(billon.voxelWidth()*cosBisector,2)+qPow(billon.voxelHeight()*sinBisector,2)) );
	tangentialBillon->setVoxelSize( billon.voxelWidth(),
									billon.voxelDepth(),
									billon.voxelHeight() );
	tangentialBillon->fill(_minIntensity);

	QVector3D origin(_origin);
	QVector3D initial, destination;

	const qreal semiKnotAreaWidthCoeff = widthOnTwo / static_cast<qreal>( nbSlices );
	int j, i, iStart, iEnd;

	int x0,y0,z0;
	qreal x0Dist, y0Dist, z0Dist;
	qreal xFrontTop, xFrontBottom, xBackTop, xBackBottom, yFront, yBack;

	iStart = iEnd = 0;
	initial.setZ(0.);
	for ( uint k=0 ; k<nbSlices ; ++k )
	{
		Slice &slice = tangentialBillon->slice(k);
		iEnd = qMin(qRound(semiKnotAreaWidthCoeff*k),widthOnTwo);
		iStart = -iEnd;
		for ( i=iStart ; i<iEnd ; ++i )
		{
			initial.setX(i);
			for ( j=-heightOnTwo ; j<heightOnTwo ; ++j )
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
						slice(j+heightOnTwo,i+widthOnTwo) = (1.-z0Dist)*yFront + z0Dist*yBack;
					}
					else
					{
						// Rotation de 90° dans le sens horaire pour correspondre à l'orientation de l'article
						slice(j+heightOnTwo,i+widthOnTwo) =	billon(y0,x0,z0);
					}
				}
			}
		}
		origin += _shiftStep;
	}

	return tangentialBillon;
}
