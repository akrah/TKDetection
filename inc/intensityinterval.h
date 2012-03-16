#ifndef INTENSITYINTERVAL_H
#define INTENSITYINTERVAL_H

#include "interval.h"

class IntensityInterval : public Interval
{
public:
	IntensityInterval();
	IntensityInterval( const int &min, const int &max );
	bool isValid() const;
};

#endif // INTENSITYINTERVAL_H
