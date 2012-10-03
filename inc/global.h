#ifndef GLOBAL_H
#define GLOBAL_H

#include <qmath.h>
#include <ostream>
#include <iostream>

#include <QVector>
#include <QVector2D>
#include <QVector3D>

/*************************************
 * Constantes d'angles
 *************************************/
#define PI					(4.*atan(1.0))
#define TWO_PI				(2.*PI)
#define PI_ON_TWO			(PI/2.)
#define PI_ON_THREE			(PI/3.)
#define PI_ON_FOUR			(PI/4.)
#define PI_ON_EIGHT			(PI/8.)
#define THREE_PI_ON_TWO		(3.*PI_ON_TWO)
#define THREE_PI_ON_FOUR	(3.*PI_ON_FOUR)
#define TWO_PI_ON_THREE		(2.*PI_ON_THREE)
#define SEVEN_PI_ON_EIGHT	(7.*PI_ON_EIGHT)
#define DEG_TO_RAD_FACT		(PI/180.)
#define RAD_TO_DEG_FACT		(180./PI)

/*************************************
 * Paramètres par défaut
 *************************************/

// Paramètres de détection des maximums et intervalles dans un histogramme
#define DEFAULT_MASK_RADIUS 2
#define DEFAULT_MINIMUM_WIDTH_OF_NEIGHBORHOOD 10
#define DEFAULT_MINIMUM_WIDTH_OF_INTERVALS 10
#define DEFAULT_PERCENTAGE_FOR_MAXIMUM_CANDIDATE 30

// Paramètres d'intensité
#define MINIMUM_INTENSITY -900
#define MAXIMUM_INTENSITY 530

// Paramètres de z-mouvement
#define MINIMUM_Z_MOTION 200
#define MAXIMUM_Z_MOTION 500

// Paramètres de détection de la moelle
#define FALSE_CUT_PERCENT 20
#define NEIGHBORHOOD_WINDOW_WIDTH 59
#define NEIGHBORHOOD_WINDOW_HEIGHT NEIGHBORHOOD_WINDOW_WIDTH
#define BINARIZATION_THRESHOLD 90
#define MARROW_LAG NEIGHBORHOOD_WINDOW_WIDTH/2

// Paramètres de flots optiques
#define FLOW_ALPHA_DEFAULT 7
#define FLOW_EPSILON_DEFAULT 0.001f
#define FLOW_MAXIMUM_ITERATIONS 100

// Type de lissage
namespace SmoothingType {
	enum SmoothingType {
		_SMOOTHING_TYPE_MIN_ = -1,
		NONE,
		MEANS,
		GAUSSIAN,
		_SMOOTHING_TYPE_MAX_
	};
}

// Types de coupe possibles
namespace SliceType {
	enum SliceType {
		_SLICE_TYPE_MIN_ = -1,
		CURRENT,
		MOVEMENT,
		EDGE_DETECTION,
		FLOW,
		RESTRICTED_AREA,
		AVERAGE,
		MEDIAN,
		_SLICE_TYPE_MAX_
	};
}

// Type de detection de contours
namespace EdgeDetectionType {
	enum EdgeDetectionType {
		_EDGE_DETECTION_MIN_ = -1,
		SOBEL,
		LAPLACIAN,
		CANNY,
		_EDGE_DETECTION_MAX_
	};
}


typedef QVector< QVector< QVector3D > > PartialDerivatives;
typedef QVector< QVector< QVector2D > > VectorsField;

/*************************************
 * Structures
 *************************************/
template<typename T> struct coord2d;
template<typename T> struct vec2d;
template<typename T> struct coord3d;

typedef struct coord2d<int> iCoord2D;
typedef struct coord2d<qreal> rCoord2D;
typedef struct vec2d<int> iVec2D;
typedef struct vec2d<qreal> rVec2D;
typedef struct coord3d<int> iCoord3D;
typedef struct coord3d<qreal> rCoord3D;

namespace
{
	inline qreal ANGLE( const qreal &xo, const qreal &yo, const qreal &x2, const qreal &y2 ) {
		const qreal x_diff = x2-xo;
		const qreal distance = qSqrt(qPow(x_diff,2)+pow(y2-yo,2));
		qreal arcos = !qFuzzyIsNull(distance)?qAcos(x_diff / distance):0;
		if ( yo > y2 ) arcos = -arcos+TWO_PI;
		return arcos;
	}

	template <typename T>
	inline qreal ANGLE( const coord2d<T> &origin, const coord2d<T> &end ) {
		const qreal distance = origin.euclideanDistance(end);
		qreal arcos = !qFuzzyIsNull(distance)?qAcos((end.x-origin.x) / distance):0;
		if ( origin.y > end.y ) arcos = -arcos+TWO_PI;
		return arcos;
	}

	template<typename T>
	inline T RESTRICT_TO( const T min, const T value, const T max ) {
		return value<min||value>max?min:value;
	}

	inline int MOD ( int x , int y )
	{
		return x >= 0 ? x % y : y - 1 - ((-x-1) % y) ;
	}
}

template<typename T>
struct coord2d
{
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d( T x, T y ) : x(x), y(y) {}
	coord2d( const coord2d<T> & other ) : x(other.x), y(other.y) {}
	coord2d( const vec2d<T> & other ) : x(other.x), y(other.y) {}
	inline bool operator ==( const coord2d<T> & other ) const { return other.x == x && other.y == y; }
	inline bool operator !=( const coord2d<T> & other ) const { return other.x != x || other.y != y; }
	inline coord2d<T> & operator += ( const coord2d<T> & other ) { x += other.x; y += other.y ; return *this; }
	inline coord2d<T> & operator *= ( qreal fact ) { x *= fact; y *= fact ; return *this; }
	inline coord2d<T> & operator -= ( const coord2d<T> & other ) { x -= other.x; y -= other.y ; return *this; }
	inline coord2d<T> & operator /= ( qreal div ) { x /= div; y /= div ; return *this; }
	inline void print( std::ostream &flux ) const { return flux << "( " << x << ", " << y << " )"; }
	inline qreal euclideanDistance( const coord2d &other ) const { return qSqrt( qPow(x-other.x,2) + qPow(y-other.y,2) ); }
	// Angle orienté du vecteur (this,secondCoord) vers le vecteur (this,thirdCoord).
	// Il est compris entre -PI et PI.
	qreal angle( const coord2d &secondCoord, const coord2d &thirdCoord ) const
	{
		return vec2d<T>(secondCoord.x-x,secondCoord.y-y).angle(vec2d<T>(thirdCoord.x-x,thirdCoord.y-y));
	}
	// Angle orienté de l'axe des abscisses vers le vecteur (this,other).
	// Il est compris entre -PI et PI.
	qreal angle( const coord2d &other ) const
	{
		const qreal distance = this->euclideanDistance(other);
		return qFuzzyIsNull(distance) ? 0. : y < other.y ? qAcos((other.x-x) / distance) : -qAcos((other.x-x) / distance);
	}
};

template<typename T>
struct vec2d : public coord2d<T>
{
	vec2d() : coord2d<T>((T)0,(T)0) {}
	vec2d( T dx, T dy ) : coord2d<T>(dx,dy) {}
	vec2d( const vec2d<T> & other ) : coord2d<T>(other) {}
	vec2d( const coord2d<T> & other ) : coord2d<T>(other) {}
	inline qreal dotProduct( const vec2d<T> & other ) const { return this->x*other.x + this->y*other.y; }
	inline qreal norm() const { return qSqrt( qPow(this->x,2) + qPow(this->y,2) ); }
	qreal angle( const vec2d<T> &other ) const
	{
		const qreal cosinus = ( this->dotProduct(other) ) / ( this->norm() * other.norm() );
		// Angle orienté en fonction du sinus = this->x * other.y - this.y * other.x
		return ( this->x * other.y - this->y * other.x ) > T() ? qAcos(cosinus) : -qAcos(cosinus);
	}
};

template <typename T>
inline const coord2d<T> operator +( const coord2d<T> & first, const coord2d<T> & other ) { return coord2d<T>(first.x+other.x,first.y+other.y); }
template <typename T>
inline const coord2d<T> operator *( const coord2d<T> & coord, qreal fact ) { return coord2d<T>(coord.x*fact,coord.y*fact); }
template <typename T>
inline const coord2d<T> operator -( const coord2d<T> & coord, const coord2d<T> & other ) { return coord2d<T>(coord.x-other.x,coord.y-other.y); }
template <typename T>
inline const coord2d<T> operator /( const coord2d<T> & coord, qreal fact ) { return coord2d<T>(coord.x/fact,coord.y/fact); }

template <typename T>
std::ostream & operator<< ( std::ostream &flux , const coord2d<T> & coord )
{
	coord.print(flux);
	return flux;
}

template<typename T>
struct coord3d
{
	T x, y, z;
	coord3d() : x((T)0), y((T)0), z((T)0) {}
	coord3d( T x, T y, T z) : x(x), y(y), z(z) {}
	inline bool operator ==( const coord3d<T> & other ) const { return other.x == x && other.y == y && other.z = z; }
	inline bool operator !=( const coord3d<T> & other ) const { return other.x != x || other.y != y || other.z != z; }
	inline coord2d<T> & operator /= ( qreal div ) { x /= div; y /= div; z /= div; return *this; }
	inline coord2d<T> & operator *= ( qreal fact ) { x *= fact; y *= fact; z *= fact; return *this; }
	inline void print( std::ostream &flux ) const { return flux << "( " << x << ", " << y << ", " << z << " )"; }
	inline qreal euclideanDistance( const coord3d &other ) const { return qSqrt( qPow(x-other.x,2) + qPow(y-other.y,2) + qPow(z-other.z,2) ); }
};

template <typename T>
std::ostream & operator<< ( std::ostream &flux , const coord3d<T> & coord ) {
	coord.print(flux);
	return flux;
}

#endif // GLOBAL_H
