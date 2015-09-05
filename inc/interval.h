#ifndef INTERVAL_H
#define INTERVAL_H

/*######################################################
  # DECLARATION
  ######################################################*/

template <typename T>
class Interval
{

public:
	Interval();
	Interval( T min, T max );
	Interval( const Interval &interval );
	virtual ~Interval();

	const T &min() const;
	const T &max() const;
	const T mid() const;
	T width() const;
	T count() const;
	bool containsClosed( const T & value ) const;
	bool containsClosed( const Interval & interval ) const;
	bool containsOpen( const T & value ) const;
	bool containsOpen( const Interval & interval ) const;
	bool intersect( const Interval &interval ) const;
	Interval<T> looped( const T &upperBound ) const;

	Interval<T>& operator =( const Interval& other );

	void setMin( const T &min );
	void setMax( const T &max );
	void setBounds( const T &min, const T &max );

//	bool isValid() const;

protected:
	T _min;
	T _max;
};

template <typename T>
bool operator ==( Interval<T> first, Interval<T> second )
{
	return !(first != second);
}

template <typename T>
bool operator !=( Interval<T> first, Interval<T> second )
{
	return (first.min() != second.min()) || (first.max() != second.max());
}

template <typename T>
Interval<T> operator +( Interval<T> first, T shift )
{
	return Interval<T>(first.min()+shift,first.max()+shift);
}

/*######################################################
  # IMPLEMENTATION
  ######################################################*/

/**********************************
 * Public constructors/destructors
 **********************************/

template <typename T> Interval<T>::Interval() : _min(T()), _max(T()) {}
template <typename T> Interval<T>::Interval( T min, T max ) : _min(min), _max(max) {}
template <typename T> Interval<T>::Interval( const Interval &interval ) : _min(interval._min), _max(interval._max) {}
template <typename T> Interval<T>::~Interval() {}

/**********************************
 * Public getters
 **********************************/

template <typename T>
inline const T & Interval<T>::min() const
{
	return _min;
}

template <typename T>
inline const T & Interval<T>::max() const
{
	return _max;
}

template <typename T>
inline const T Interval<T>::mid() const
{
	return (_min + _max)/2.;
}

template <typename T>
inline T Interval<T>::width() const
{
	return _max - _min;
}

template <typename T>
inline T Interval<T>::count() const
{
	return _max - _min + 1;
}

template <typename T>
inline bool Interval<T>::containsClosed( const T &value ) const
{
	return (value >= _min) && (value <= _max);
}

template <typename T>
inline bool Interval<T>::containsClosed( const Interval &interval ) const
{
	return (interval._min >= _min) && (interval._max <= _max);
}

template <typename T>
inline bool Interval<T>::containsOpen( const T &value ) const
{
	return (value > _min) && (value < _max);
}

template <typename T>
inline bool Interval<T>::containsOpen( const Interval &interval ) const
{
	return (interval._min > _min) && (interval._max < _max);
}

template <typename T>
bool Interval<T>::intersect( const Interval &interval ) const
{
	return !((interval._max < _min) || (interval._min > _max));
}

template <typename T>
Interval<T> Interval<T>::looped( const T &upperBound ) const
{
	return Interval<T>(_min,_max-upperBound);
}

/**********************************
 * Public setters
 **********************************/

template <typename T>
Interval<T> &Interval<T>::operator =( const Interval<T>& other )
{
	_min = other._min;
	_max = other._max;
	return *this;
}

template <typename T>
void Interval<T>::setMin( const T &min )
{
	_min = min;
}

template <typename T>
void Interval<T>::setMax( const T &max )
{

	_max = max;
}

template <typename T>
void Interval<T>::setBounds( const T &min, const T &max )
{
	_min = min;
	_max = max;
}

#endif // INTERVAL_H
