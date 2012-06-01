#ifndef INTERVAL_H
#define INTERVAL_H

#include <QObject>

class Interval : public QObject
{
	Q_OBJECT

public:
	Interval();
	Interval( const int &minValue, const int &maxValue );
	Interval( const Interval &interval );

	int minValue() const;
	int maxValue() const;
	int count() const;
	int size() const;
	int width() const;
	bool contains( const int &value ) const;
	bool contains( const Interval &interval ) const;
	bool containsOpen( const int &value ) const;
	bool containsOpen( const Interval &interval ) const;
	bool containsClosed( const int &value ) const;
	bool containsClosed( const Interval &interval ) const;

	bool isValid() const;

	Interval inverted() const;
	Interval& operator =(const Interval& other);

public slots:
	void setMin( const int &minValue );
	void setMax( const int &maxValue );
	void setBounds( const int &minValue, const int &maxValue );

protected:
	int _min;
	int _max;
};

#endif // INTERVAL_H
