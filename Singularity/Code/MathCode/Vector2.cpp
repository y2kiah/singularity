//	----==== VECTOR2.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		3
//	Date:			5/04
//	Description:	A highly optimized 2D vector class
//	--------------------------------------------------------------------------------


#include <sstream>
#include "vector2.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Vector2 //////////


void Vector2::normalize(void)
{
	// Normalize the vector to unit length
	float magSq = this->magSquared();

	if (magSq > 0) {
		float invMag = 1.0f / sqrtf(magSq);
		x *= invMag;
		y *= invMag;
	}
}


std::string Vector2::toString(void) const
{
	std::ostringstream returnStr;
	returnStr << "(" << x << "," << y << ")";
	return returnStr.str();
}