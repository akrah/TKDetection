#ifndef SLICESINTERVAL_H
#define SLICESINTERVAL_H

#include "interval.h"

class SlicesInterval : public Interval
{
public:
	SlicesInterval();
	SlicesInterval( const int &min, const int &max );
	bool isValid() const;
	SlicesInterval& operator =(const SlicesInterval& other);
};

#endif // SLICESINTERVAL_H
