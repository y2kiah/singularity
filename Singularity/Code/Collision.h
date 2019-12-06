/* ----==== COLLISION.H ====---- */

#ifndef COLLISION_H
#define COLLISION_H

#include "MATHCODE\plane3.h"
#include "MATHCODE\vector3.h"

/*---------------
---- DEFINES ----
---------------*/

/*
Return Value Info

0 = no collision
1 = partial collision or collision
2 = total collision
3 = error
*/

enum colTestType {
	COLLISION_FALSE = 0,
	COLLISION_TRUE,
	COLLISION_FULL_INSIDE,
	COLLISION_ERROR
};

/*------------------
---- STRUCTURES ----
------------------*/

class BSphere;
class colAABB;
class colOBB;
class Frustum;

class colVolume {
	public:
		virtual colTestType collideWithPoint(const Vector3 &) const = 0;
		virtual colTestType collideWithPlane(const Plane3 &) const = 0;
		virtual colTestType collideWithSphere(const BSphere &) const = 0;
//		virtual colTestType collideWithAABB(colAABB *) const = 0;
//		virtual colTestType collideWithOBB(colOBB *) const = 0;
		virtual colTestType collideWithFrustum(const Frustum &) const = 0;

		explicit colVolume() {}
		virtual ~colVolume() {}
};

class BSphere : public colVolume {
	public:
		Vector3		p;
		float		radius;

		colTestType	collideWithPoint(const Vector3 &) const;
		colTestType collideWithPlane(const Plane3 &) const;
		colTestType collideWithSphere(const BSphere &) const;
//		colTestType collideWithAABB(colAABB *) const;
//		colTestType collideWithOBB(colOBB *) const;
		colTestType collideWithFrustum(const Frustum &) const { return COLLISION_FALSE; }

		explicit BSphere() : p(0,0,0), radius(0) {}
		explicit BSphere(const Vector3 &v, float r) : p(v), radius(r) {}
		explicit BSphere(float x, float y, float z, float r) : p(x,y,z), radius(r) {}
};

class colAABBi : public colVolume {
	public:
		int		xpos[2];
		int		ypos[2];
		int		zpos[2];

		colTestType	collideWithPoint(const Vector3 &) const;
		colTestType collideWithPlane(const Plane3 &) const { return COLLISION_FALSE; }
		colTestType collideWithSphere(const BSphere &) const { return COLLISION_FALSE; }
//		colTestType collideWithAABB(colAABB *) const;
//		colTestType collideWithOBB(colOBB *) const;
		colTestType collideWithFrustum(const Frustum &) const;

		void draw(float r, float g, float b) const;

		explicit colAABBi(int x1, int x2, int y1, int y2, int z1, int z2);
		explicit colAABBi() { xpos[0] = xpos[1] = ypos[0] = ypos[1] = zpos[0] = zpos[1] = 0; }
};

class colAABBf : public colVolume {
	private:
		float	xpos[2];
		float	ypos[2];
		float	zpos[2];

	public:
		colTestType	collideWithPoint(const Vector3 &) const;
		colTestType collideWithPlane(const Plane3 &) const { return COLLISION_FALSE; }
		colTestType collideWithSphere(const BSphere &) const { return COLLISION_FALSE; }
//		colTestType collideWithAABB(colAABB *) const;
//		colTestType collideWithOBB(colOBB *) const;
		colTestType collideWithFrustum(const Frustum &) const;

		explicit colAABBf(Vector3 &ul,Vector3 &lr);		// upper left and lower right
		explicit colAABBf(float x1, float x2, float y1, float y2, float z1, float z2);
		explicit colAABBf() { xpos[0] = xpos[1] = ypos[0] = ypos[1] = zpos[0] = zpos[1] = 0; }
};

class colOBB : public colVolume {
};

/*class AABB {
	public:
		Vector3		p[8];
		Plane3D		plane[6];
};

class OBB {
	public:
		Vector3		op[8], rp[8];		// Original points and rotated points
		Plane3D		plane[6];

		void Reset(void) {}
		void Rotate(int _xangle, int _yangle, int _zangle) {}
};*/

#endif