#ifndef COORDINATE_H
#define COORDINATE_H

#include <qmath.h>
#include <ostream>

template<typename T>
struct coord2d
{
	T x, y;
	coord2d() : x((T)0), y((T)0) {}
	coord2d( T x, T y ) : x(x), y(y) {}
	template<typename U>
	coord2d( const coord2d<U> & other ) : x(other.x), y(other.y) {}
	coord2d( const vec2d<T> & other ) : x(other.x), y(other.y) {}
	inline bool operator ==( const coord2d<T> & other ) const { return other.x == x && other.y == y; }
	inline bool operator !=( const coord2d<T> & other ) const { return other.x != x || other.y != y; }
	inline coord2d<T> & operator += ( const coord2d<T> & other ) { x += other.x; y += other.y ; return *this; }
	inline coord2d<T> & operator *= ( qreal fact ) { x *= fact; y *= fact ; return *this; }
	inline coord2d<T> & operator -= ( const coord2d<T> & other ) { x -= other.x; y -= other.y ; return *this; }
	inline coord2d<T> & operator /= ( qreal div ) { x /= div; y /= div ; return *this; }
	inline void print( std::ostream &flux ) const { return flux << "( " << x << ", " << y << " )"; }
	inline qreal euclideanDistance( const coord2d &other ) const { return qSqrt( qPow((qreal)(x)-(qreal)(other.x),2) + qPow((qreal)(y)-(qreal)(other.y),2) ); }
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
		return qFuzzyIsNull(distance) ? 0. : (qreal)(y) < (qreal)(other.y) ? qAcos(((qreal)(other.x)-(qreal)(x)) / distance) : -qAcos(((qreal)(other.x)-(qreal)(x)) / distance);
	}
	qreal vectorProduct( const coord2d &secondCoord, const coord2d &thirdCoord ) const
	{
		return (secondCoord.x-x)*(thirdCoord.y-y) - (thirdCoord.x-x)*(secondCoord.y-y);
	}
};

template<typename T>
static bool lessThanX( const coord2d<T> &left, const coord2d<T> &right )
{
	return left.x < right.x;
}

template<typename T>
static bool lessThanY( const coord2d<T> &left, const coord2d<T> &right )
{
	return left.y < right.y;
}

static coord2d<int> invalidICoord2D = coord2d<int>(-1,-1);

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
template <typename T, typename U>
inline const coord2d<T> operator +( const coord2d<T> & first, const coord2d<U> & other ) { return coord2d<T>(first.x+other.x,first.y+other.y); }
template <typename T>
inline const coord2d<T> operator *( const coord2d<T> & coord, const qreal & fact ) { return coord2d<T>(coord.x*fact,coord.y*fact); }
template <typename T>
inline const coord2d<T> operator *( const coord2d<T> & coord, const coord2d<T> & other ) { return coord2d<T>(coord.x*other.x,coord.y*other.y); }
template <typename T, typename U>
inline const coord2d<T> operator *( const coord2d<T> & coord, const coord2d<U> & other ) { return coord2d<T>(coord.x*other.x,coord.y*other.y); }
template <typename T>
inline const coord2d<T> operator -( const coord2d<T> & coord, const coord2d<T> & other ) { return coord2d<T>(coord.x-other.x,coord.y-other.y); }
template <typename T, typename U>
inline const coord2d<T> operator -( const coord2d<T> & coord, const coord2d<U> & other ) { return coord2d<T>(coord.x-other.x,coord.y-other.y); }
template <typename T>
inline const coord2d<T> operator /( const coord2d<T> & coord, const qreal & fact ) { return coord2d<T>(coord.x/fact,coord.y/fact); }
template <typename T, typename U>
inline const coord2d<U> operator /( const coord2d<T> & coord, const coord2d<U> & other ) { return coord2d<U>(coord.x/other.x,coord.y/other.y); }

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
inline const coord3d<T> operator +( const coord3d<T> & first, const coord3d<T> & other ) { return coord3d<T>(first.x+other.x,first.y+other.y,first.z+other.z); }

template <typename T>
std::ostream & operator<< ( std::ostream &flux , const coord3d<T> & coord )
{
	coord.print(flux);
	return flux;
}

#endif // COORDINATE_H
