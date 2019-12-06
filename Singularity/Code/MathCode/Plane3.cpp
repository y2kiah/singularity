//	----==== PLANE3.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			5/04
//	Description:	Represents a 3d plane by storing its normal and distance from the
//					origin (or d in the plane equation)
//	---------------------------------------------------------------------------


#include "plane3.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Plane3 //////////


void Plane3::set(const Vector3 &p1,const Vector3 &p2,const Vector3 &p3)
{
	Vector3 one(p2);
	one -= p1;						// one = p2 - p1;
		
	Vector3 two(p3);
	two -= (p1);					// two = p3 - p1;

	n.unitNormalOf(one, two);		// Get the unit normal
	d = n * p1;						// Get the plane equation constant with dot product
}


void Plane3::set(const Vector3 &p, const Vector3 &_n)
{
	n = _n;
	n.normalize();

	d = n * p;
}


Plane3::Plane3(float x1,float y1,float z1,float x2,float y2,float z2,float x3,float y3,float z3)
{
	Vector3 p1(x1, y1, z1);

	Vector3 one(x2-x1, y2-y1, z2-z1);	// one = p2 - p1;
	Vector3 two(x3-x1, y3-y1, z3-z1);	// two = p3 - p1;

	n.unitNormalOf(one, two);		// Get the unit normal
	d = n * p1;						// Get the plane equation constant with dot product
}


Plane3::Plane3(const Vector3 &p1,const Vector3 &p2,const Vector3 &p3)
{
	Vector3 one(p2);
	one -= p1;						// one = p2 - p1;
		
	Vector3 two(p3);
	two -= (p1);					// two = p3 - p1;

	n.unitNormalOf(one, two);		// Get the unit normal
	d = n * p1;						// Get the plane equation constant with dot product
}


Plane3::Plane3(const Vector3 &_n, const Vector3 &_p, bool unitNormal)
{
	n = _n;
	if (!unitNormal) n.normalize();

	d = n * _p;
}
