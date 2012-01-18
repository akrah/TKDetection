#include "inc/marrow_def.h"

std::ostream& operator<<( std::ostream &flux, Coord2D const& coord )
{
	flux << "( " << coord.x << ", " << coord.y << " )";
	return flux;
}
