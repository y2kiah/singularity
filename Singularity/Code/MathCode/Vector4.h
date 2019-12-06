//	----==== VECTOR4.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		3
//	Date:			4/04
//	Description:	4D vector class
//	--------------------------------------------------------------------------------


#ifndef VECTOR4_H
#define VECTOR4_H

#include <string>
#include <cmath>
#include "..\UTILITYCODE\msgassert.h"
#include "..\UTILITYCODE\typedefs.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Vector3;
class Matrix4x4;


class Vector4 {

	public:

		///// Variables
		
		union {
			struct {
				float x, y, z, w;	// This union allows access to the same memory location through either
			};						// x,y,z,w or the array v[4]. Use v for fast OpenGL functions

			float v[4];
		};


		///// Overloaded Operators

		// Conditional operators
		bool	operator==(const Vector4 &p) const { return (x == p.x && y == p.y && z == p.z && w == p.w); }
		bool	operator!=(const Vector4 &p) const { return (x != p.x || y != p.y || z != p.z || w != p.w); }

		// Assignment operators
		void	operator= (const Vector4 &p) { x = p.x; y = p.y; z = p.z; w = p.w; }
		void	operator+=(const Vector4 &p) { x += p.x; y += p.y; z += p.z; w += p.w; }
		void	operator-=(const Vector4 &p) { x -= p.x; y -= p.y; z -= p.z; w -= p.w; }
		void	operator*=(float s) { x *= s; y *= s; z *= s; w *= s; }
		void	operator*=(const Matrix4x4 &m);
		void	operator/=(float s) { msgAssert(s != 0, "vector divide by 0"); s = 1.0f / s; x *= s; y *= s; z *= s; w *= s; }

		// Not as fast as functions below, but these allow streamed operations ie. V = VA + VB - VC
		Vector4 operator- (void) const { return Vector4(-x,-y,-z,-w); }
		Vector4	operator+ (const Vector4 &p) const { return Vector4(x+p.x, y+p.y, z+p.z, w+p.w); }
		Vector4	operator- (const Vector4 &p) const { return Vector4(x-p.x, y-p.y, z-p.z, w-p.w); }
		Vector4	operator* (float s) const { return Vector4(x*s, y*s, z*s, w*s); }
		Vector4	operator/ (float s) const { msgAssert(s != 0, "vector divide by 0"); s = 1.0f / s; return Vector4(x*s, y*s, z*s, w*s); }

		Vector4	operator* (const Matrix4x4 &m) const;

		// Dot product
		float	operator* (const Vector4 &p) const { return (x * p.x) + (y * p.y) + (z * p.z) + (w * p.w); }

		///// Functions

		// Convert from 3d vector to 4d vector, often w = 0 used for vectors and w = 1 used for points
		// when multiplying by matrix, w = 1 means it can be translated from origin
		void			assign(const Vector4 &p) { x = p.x; y = p.y; z = p.z; w = p.w; }
		void			assign(const Vector3 &p, float _w);
		void			assign(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

		// Check equality with floating point error, SLOW
		__inline bool	equalTo(const Vector4 &p) const;
		__inline bool	notEqualTo(const Vector4 &p) const;

		// Faster than the overloaded operators above		
		__inline void	add(const Vector4 &p1, const Vector4 &p2);
		__inline void	subtract(const Vector4 &p1, const Vector4 &p2);
		__inline void	multiply(const Vector4 &p, float s);
		__inline void	divide(const Vector4 &p, float s);
		
		void			multiplyMatrix(const Vector4 &p, const Matrix4x4 &m);

		// Distance
		__inline float	dist(const Vector4 &p) const;
		__inline float	distSquared(const Vector4 &p) const;

		// Finds the magnitude of the vector, or it's distance from the origin
		float			mag(void) const { return sqrtf(x*x + y*y + z*z + w*w); }
		float			magSquared(void) const { return x*x + y*y + z*z + w*w; }

		// Vector methods
		void			normalize(void);
		
		// Debugging
		std::string		toString(void) const;

		// Constructors / Destructor
		Vector4() : x(0), y(0), z(0), w(0) {}
		Vector4(const Vector4 &p) : x(p.x), y(p.y), z(p.z), w(p.w) {}
		explicit		Vector4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
		explicit		Vector4(const Vector3 &p, float _w);
		~Vector4() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/


__inline bool Vector4::equalTo(const Vector4 &p) const
{
	if (fabs(p.x - x) <= EPSILON)
		if (fabs(p.y - y) <= EPSILON)
			if (fabs(p.z - z) <= EPSILON)
				if (fabs(p.w - w) <= EPSILON)
					return true;
	
	return false;
}


__inline bool Vector4::notEqualTo(const Vector4 &p) const
{
	if ((fabs(p.x - x) > EPSILON) || (fabs(p.y - y) > EPSILON) || (fabs(p.z - z) > EPSILON) || (fabs(p.w - w) > EPSILON))
		return true;
	else
		return false;
}


__inline void Vector4::add(const Vector4 &p1,const Vector4 &p2)
{
	x = p1.x + p2.x;
	y = p1.y + p2.y;
	z = p1.z + p2.z;
	w = p1.w + p2.w;
}


__inline void Vector4::subtract(const Vector4 &p1,const Vector4 &p2)
{
	x = p1.x - p2.x;
	y = p1.y - p2.y;
	z = p1.z - p2.z;
	w = p1.w - p2.w;
}


__inline void Vector4::multiply(const Vector4 &p, float s)
{
	x = p.x * s;
	y = p.y * s;
	z = p.z * s;
	w = p.w * s;
}


__inline void Vector4::divide(const Vector4 &p, float s)
{
	msgAssert(s != 0, "vector divide by 0");
	s = 1.0f / s;
	x = p.x * s;
	y = p.y * s;
	z = p.z * s;
	w = p.w * s;
}


__inline float Vector4::dist(const Vector4 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	const float dz = p.z - z;
	const float dw = p.w - w;
	return sqrtf(dx*dx + dy*dy + dz*dz + dw*dw);
}


__inline float Vector4::distSquared(const Vector4 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	const float dz = p.z - z;
	const float dw = p.w - w;
	return dx*dx + dy*dy + dz*dz + dw*dw;
}


#endif