//	----==== PLANE3.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			5/04
//	Description:	Represents a 3d plane by storing its normal and distance from the
//					origin (or d in the plane equation)
//----------------------------------------------------------------------------------------


#ifndef PLANE3_H
#define PLANE3_H

#include "vector3.h"

/*------------------
---- STRUCTURES ----
------------------*/


class Plane3 {
	public:

		///// Variables

		Vector3		n;			// plane normal
		float		d;			// ax + by + cz + d = 0 ... or distance from origin of plane

		///// Functions

		float		findDist(const Vector3 &_p) const { return (n * _p) - d; }
		void		set(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
		void		set(const Vector3 &p, const Vector3 &n);

		// Constructors / Destructor

		explicit	Plane3() : n(0,0,0), d(0) {}
		explicit	Plane3(float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3);
		explicit	Plane3(const Vector3 &p1, const Vector3 &p2, const Vector3 &p3);
		explicit	Plane3(const Vector3 &_n, const Vector3 &_p, bool unitNormal);
		explicit	Plane3(const Vector3 &_n, float _d) : n(_n), d(_d) { n.normalize(); }
		explicit	Plane3(const Plane3 &p) : n(p.n), d(p.d) {}
		~Plane3() {}
};

#endif