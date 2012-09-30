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

	T min() const;
	T max() const;
	T size() const;
	T width() const;
	bool containsOpen( const T &value ) const;
	bool containsOpen( const Interval &interval ) const;
	bool containsClosed( const T &value ) const;
	bool containsClosed( const Interval &interval ) const;
	bool isValid() const;
	Interval<T> inverted() const;

	Interval<T>& operator =( const Interval& other );

	void setMin( const T &min );
	void setMax( const T &max );
	void setBounds( const T &min, const T &max );

protected:
	T _min;
	T _max;
};

/*######################################################
  # INSTANCIATION
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
inline T Interval<T>::min() const
{
	return _min;
}

template <typename T>
inline T Interval<T>::max() const
{
	return _max;
}

template <typename T>
inline T Interval<T>::size() const
{
	return _max - _min;
}

template <typename T>
inline T Interval<T>::width() const
{
	return size();
}

template <typename T>
inline bool Interval<T>::containsOpen( const T &value ) const
{
	return (value > _min) && (value < _max);
}

template <typename T>
inline bool Interval<T>::containsOpen( const Interval &interval ) const
{
	return (_min < interval._min) && (_max > interval._max);
}

template <typename T>
inline bool Interval<T>::containsClosed( const T &value ) const
{
	return (value >= _min) && (value <= _max);
}

template <typename T>
inline bool Interval<T>::containsClosed( const Interval &interval ) const
{
	return (_min <= interval._min) && (_max >= interval._max);
}

template <typename T>
inline bool Interval<T>::isValid() const
{
	return _min<=_max;
}

template <typename T>
inline Interval<T> Interval<T>::inverted() const
{
	return Interval(_max,_min);
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
	setMin(min);
	setMax(max);
}

#endif // INTERVAL_H
