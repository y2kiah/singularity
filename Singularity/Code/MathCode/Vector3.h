//	----==== VECTOR3.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		4
//	Date:			5/04
//	Description:	3D vector class
//
//	External Dependencies:	requires the class LookupManager to be resident in
//							memory for trig lookup tables
//	--------------------------------------------------------------------------------


#ifndef VECTOR3_H
#define VECTOR3_H

#include <string>
#include <cmath>
#include "..\UTILITYCODE\msgassert.h"
#include "..\UTILITYCODE\typedefs.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Vector4;
class Matrix4x4;


class Point3i {
	public:
		int x, y, z;

		explicit Point3i() : x(0), y(0), z(0) {}
		explicit Point3i(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}
};


class Vector3 {

	public:

		///// Variables
		
		union {
			struct {
				float x, y, z;		// This union allows access to the same memory location through either
			};						// x,y,z or the array v[3]. Use v for fast OpenGL functions

			float v[3];
		};


		///// Overloaded Operators

		// Conditional operators
		bool	operator==(const Vector3 &p) const { return (x == p.x && y == p.y && z == p.z); }
		bool	operator!=(const Vector3 &p) const { return (x != p.x || y != p.y || z != p.z); }

		// Assignment operators
		void	operator= (const Vector3 &p) { x = p.x; y = p.y; z = p.z; }
		void	operator+=(const Vector3 &p) { x += p.x; y += p.y; z += p.z; }
		void	operator-=(const Vector3 &p) { x -= p.x; y -= p.y; z -= p.z; }
		void	operator*=(float s) { x *= s; y *= s; z *= s; }
		void	operator*=(const Matrix4x4 &m);
		void	operator/=(float s) { msgAssert(s != 0, "vector divide by 0"); s = 1.0f / s; x *= s; y *= s; z *= s; }

		// Not as fast as functions below, but these allow streamed operations ie. V = VA + VB - VC
		Vector3	operator- (void) const { return Vector3(-x,-y,-z); }
		Vector3	operator+ (const Vector3 &p) const { return Vector3(x+p.x, y+p.y, z+p.z); }
		Vector3	operator- (const Vector3 &p) const { return Vector3(x-p.x, y-p.y, z-p.z); }
		Vector3	operator* (float s) const {	return Vector3(x*s, y*s, z*s); }
		Vector3	operator/ (float s) const { msgAssert(s != 0, "vector divide by 0");	s = 1.0f / s; return Vector3(x*s, y*s, z*s); }

		Vector3	operator* (const Matrix4x4 &m) const;	// Use 3x3 portion of matrix

		// Dot product
		float	operator* (const Vector3 &p) const { return (x * p.x) + (y * p.y) + (z * p.z); }

		// Cross Product
		Vector3	operator% (const Vector3 &p) const { return Vector3((y*p.z)-(z*p.y),(z*p.x)-(x*p.z),(x*p.y)-(y*p.x)); }

		///// Functions

		// Convert from 4d vector to 3d vector, ignore w component of 4d vector
		void			assign(const Vector4 &p);
		void			assign(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

		// Check equality with floating point error, SLOW
		__inline bool	equalTo(const Vector3 &p) const;
		__inline bool	notEqualTo(const Vector3 &p) const;

		// Faster than the overloaded operators above
		__inline void	add(const Vector3 &p1, const Vector3 &p2);
		__inline void	subtract(const Vector3 &p1, const Vector3 &p2);
		__inline void	multiply(const Vector3 &p, float s);
		__inline void	divide(const Vector3 &p, float s);
		__inline void	crossProduct(const Vector3 &p1, const Vector3 &p2);
		
		void			multiplyMatrix(const Vector3 &p, const Matrix4x4 &m);

		// Distance
		__inline float	dist(const Vector3 &p) const;
		__inline float	distSquared(const Vector3 &p) const;

		// Finds the magnitude of the vector, or it's distance from the origin
		float			mag(void) const { return sqrtf(x*x + y*y + z*z); }
		float			magSquared(void) const { return x*x + y*y + z*z; }

		// Vector methods
		__inline void	normalOf(const Vector3 &p1, const Vector3 &p2);
		void			unitNormalOf(const Vector3 &p1, const Vector3 &p2);
		void			normalize(void);
		
		// 3D Rotation functions
		void			rot3D(Vector3 &p, int xa, int ya, int za);
		void			rot3D(int xa, int ya, int za);

		// Debugging
		std::string		toString(void) const;

		// Constructors / Destructor
		Vector3() : x(0), y(0), z(0) {}
		Vector3(const Vector3 &p) : x(p.x), y(p.y), z(p.z) {}
		explicit		Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
		explicit		Vector3(const Vector4 &p);
		~Vector3() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/


__inline bool Vector3::equalTo(const Vector3 &p) const
{
	if (fabs(p.x - x) <= EPSILON)
		if (fabs(p.y - y) <= EPSILON)
			if (fabs(p.z - z) <= EPSILON)
				return true;
	
	return false;
}


__inline bool Vector3::notEqualTo(const Vector3 &p) const
{
	if ((fabs(p.x - x) > EPSILON) || (fabs(p.y - y) > EPSILON) || (fabs(p.z - z) > EPSILON)) return true;
	else return false;
}


__inline void Vector3::add(const Vector3 &p1,const Vector3 &p2)
{
	x = p1.x + p2.x;
	y = p1.y + p2.y;
	z = p1.z + p2.z;
}


__inline void Vector3::subtract(const Vector3 &p1,const Vector3 &p2)
{
	x = p1.x - p2.x;
	y = p1.y - p2.y;
	z = p1.z - p2.z;
}


__inline void Vector3::multiply(const Vector3 &p, float s)
{
	x = p.x * s;
	y = p.y * s;
	z = p.z * s;
}


__inline void Vector3::divide(const Vector3 &p, float s)
{
	msgAssert(s != 0.0f, "vector divide by 0");
	s = 1.0f / s;
	x = p.x * s;
	y = p.y * s;
	z = p.z * s;
}


__inline void Vector3::crossProduct(const Vector3 &p1, const Vector3 &p2)
{
	// Sets point to cross product of two vectors
	x = (p1.y * p2.z) - (p1.z * p2.y),
	y = (p1.z * p2.x) - (p1.x * p2.z),
	z = (p1.x * p2.y) - (p1.y * p2.x);
}


__inline float Vector3::dist(const Vector3 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	const float dz = p.z - z;
	return sqrtf(dx*dx + dy*dy + dz*dz);
}


__inline float Vector3::distSquared(const Vector3 &p) const
{
	const float dx = p.x - x;
	const float dy = p.y - y;
	const float dz = p.z - z;
	return dx*dx + dy*dy + dz*dz;
}


__inline void Vector3::normalOf(const Vector3 &p1, const Vector3 &p2)
{
	// Calculates the normal vector with cross product
	x = (p1.y * p2.z) - (p1.z * p2.y);
	y = (p1.z * p2.x) - (p1.x * p2.z);
	z = (p1.x * p2.y) - (p1.y * p2.x);
}


#endif