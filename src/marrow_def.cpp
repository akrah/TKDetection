#include "inc/marrow_def.h"

std::ostream& operator<<( std::ostream &flux, const Coord2D & coord )
{
	flux << "( " << coord.x << ", " << coord.y << " )";
	return flux;
}
