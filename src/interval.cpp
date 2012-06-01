#include "inc/interval.h"

Interval::Interval() : QObject(), _min(0), _max(0)
{
}

Interval::Interval( const int &min, const int &max ) : QObject(), _min(min), _max(max)
{
}

Interval::Interval( const Interval &interval ) : QObject(), _min(interval._min), _max(interval._max)
{
}

int Interval::minValue() const
{
	return _min;
}

int Interval::maxValue() const
{
	return _max;
}

int Interval::size() const
{
	return _max - _min;
}

int Interval::count() const
{
	return size() + 1;
}

int Interval::width() const
{
	return size();
}

bool Interval::contains( const int &value ) const
{
	return containsClosed(value);
}

bool Interval::contains( const Interval &interval ) const
{
	return containsClosed(interval);
}

bool Interval::containsOpen( const int &value ) const
{
	return (value > minValue()) && (value < maxValue());
}

bool Interval::containsOpen( const Interval &interval ) const
{
	return (minValue() < interval.minValue()) && (maxValue() > interval.maxValue());
}

bool Interval::containsClosed( const int &value ) const
{
	return (value >= minValue()) && (value <= maxValue());
}

bool Interval::containsClosed( const Interval &interval ) const
{
	return (minValue() <= interval.minValue()) && (maxValue() >= interval.maxValue());
}

bool Interval::isValid() const
{
	return minValue()<=maxValue();
}

Interval Interval::inverted() const
{
	return Interval(maxValue(),minValue());
}

Interval& Interval::operator =(const Interval& other)
{
	_min = other._min;
	_max = other._max;
	return *this;
}

void Interval::setMin( const int &min )
{
	_min = min;
}

void Interval::setMax( const int &max )
{

	_max = max;
}

void Interval::setBounds( const int &min, const int &max )
{
	_min = min;
	_max = max;
}
