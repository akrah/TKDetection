#ifndef GLOBAL_H
#define GLOBAL_H

namespace
{
	template<typename T>
	inline T RESTRICT_TO( const T min, const T value, const T max ) {
		return value<min||value>max?min:value;
	}

	inline int MOD ( int x , int y )
	{
		return x >= 0 ? x % y : y - 1 - ((-x-1) % y) ;
	}
}

#endif // GLOBAL_H
