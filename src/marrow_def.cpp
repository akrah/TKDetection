#include "inc/marrow_def.h"

#include <iostream>

std::ostream& operator<<( std::ostream &flux, Coord2D const& coord )
{
	flux << "( " << coord.x << ", " << coord.y << " )";
	return flux;
}
