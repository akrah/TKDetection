#ifndef INTERVAL_H
#define INTERVAL_H

#include <QObject>

class Interval : public QObject
{
	Q_OBJECT

public:
	Interval();
	Interval( const int &min, const int &max );
	Interval( const Interval &interval );

	int min() const;
	int max() const;
	int size() const;
	int count() const;
	bool containsOpen( const int &value ) const;
	bool containsOpen( const Interval &interval ) const;
	bool containsClosed( const int &value ) const;
	bool containsClosed( const Interval &interval ) const;

	virtual bool isValid() const = 0;

public slots:
	void setMin( const int &min );
	void setMax( const int &max );
	void setBounds( const int &min, const int &max );

protected:
	int _min;
	int _max;
};

#endif // INTERVAL_H
