#ifndef SLICESINTERVAL_H
#define SLICESINTERVAL_H

#include <QObject>

class SlicesInterval : public QObject
{
	Q_OBJECT

public:
	SlicesInterval();
	SlicesInterval( int min, int max );

	int min() const;
	int max() const;
	bool isValid() const;

public slots:
	void setMin( const int &min );
	void setMax( const int &max );

protected:
	int _min;
	int _max;
};

#endif // SLICESINTERVAL_H
