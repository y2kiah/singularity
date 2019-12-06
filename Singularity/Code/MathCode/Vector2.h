//	----==== VECTOR2.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		3
//	Date:			5/04
//	Description:	2D vector class
//	--------------------------------------------------------------------------------


#ifndef VECTOR2_H
#define VECTOR2_H

#include <string>
#include <cmath>
#include "..\UTILITYCODE\msgassert.h"
#include "..\UTILITYCODE\typedefs.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Point2i {
	public:
		int x, y;

		explicit Point2i() : x(0), y(0) {}
		explicit Point2i(int _x, int _y) : x(_x), y(_y) {}
};


class Vector2 {
	
	public:

		///// Variables

		float x, y;

		///// Overloaded Operators

		// Conditional operators
		bool	operator==(const Vector2 &p) const { return (x == p.x && y == p.y); }
		bool	operator!=(const Vector2 &p) const { return (x != p.x || y != p.y); }

		// Assignment operators
		void	operator+=(const Vector2 &p) { x += p.x; y += p.y; }
		void	operator-=(const Vector2 &p) { x -= p.x; y -= p.y; }
		void	operator*=(float s) { x *= s; y *= s; }
		void	operator/=(float s) { msgAssert(s != 0, "vector divide by 0"); x /= s; y /= s; }

		// Not as fast as functions below, but these allow streamed operations ie. V = VA + VB - VC
		Vector2 operator- (void) const { return Vector2(-x,-y); }
		Vector2	operator+ (const Vector2 &p) const { return Vector2(x+p.x, y+p.y); }
		Vector2	operator- (const Vector2 &p) const { return Vector2(x-p.x, y-p.y); }
		Vector2	operator* (float s) const { return Vector2(x*s, y*s); }
		Vector2	operator/ (float s) const { msgAssert(s != 0, "vector divide by 0"); return Vector2(x/s, y/s); }

		// Dot product
		float	operator* (const Vector2 &p) const { return (x * p.x) + (y * p.y); }

		///// Functions

		void			assign(float _x, float _y) { x = _x; y = _y; }

		// Check equality with floating point error, SLOW
		__inline bool	equalTo(const Vector2 &p) const;
		__inline bool	notEqualTo(const Vector2 &p) const;

		// Faster than the overloaded operators above
		__inline void	add(const Vector2 &p1, const Vector2 &p2);
		__inline void	subtract(const Vector2 &p1, const Vector2 &p2);
		__inline void	multiply(const Vector2 &p, float s);
		__inline void	divide(const Vector2 &p, float s);
		
		// Finds the distance between two points
		__inline float	dist(const Vector2 &p) const;
		__inline float	distSquared(const Vector2 &p) const;

		// Finds the magnitude of the vector, or it's distance from the origin
		float			mag(void) const { return sqrtf(x*x + y*y); }
		float			magSquared(void) const { return x*x + y*y; }

		// Vector methods
		void			normalize(void);

		// Debugging
		std::string		toString(void) const;

		// Constructors / Destructor
		Vector2() : x(0), y(0) {}
		Vector2(const Vector2 &p) : x(p.x), y(p.y) {}
		explicit		Vector2(float _x, float _y) : x(_x), y(_y) {}
		~Vector2() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/


__inline bool Vector2::equalTo(const Vector2 &p) const
{
	if (fabs(p.x - x) <= EPSILON)
		if (fabs(p.y - y) <= EPSILON)
			return true;
	
	return false;
}


__inline bool Vector2::notEqualTo(const Vector2 &p) const
{
	if ((fabs(p.x - x) > EPSILON) || (fabs(p.y - y) > EPSILON)) return true;
	else return false;
}


__inline void Vector2::add(const Vector2 &p1, const Vector2 &p2)
{
	x = p1.x + p2.x;
	y = p1.y + p2.y;
}


__inline void Vector2::subtract(const Vector2 &p1, const Vector2 &p2)
{
	x = p1.x - p2.x;
	y = p1.y - p2.y;
}


__inline void Vector2::multiply(const Vector2 &p, float s)
{
	x = p.x * s;
	y = p.y * s;
}


__inline void Vector2::divide(const Vector2 &p, float s)
{
	msgAssert(s != 0, "vector divide by 0");
	x = p.x / s;
	y = p.y / s;
}


__inline float Vector2::dist(const Vector2 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	return sqrtf(dx*dx + dy*dy);
}


__inline float Vector2::distSquared(const Vector2 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	return dx*dx + dy*dy;
}


#endif