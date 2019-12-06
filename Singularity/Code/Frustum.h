/* ----==== FRUSTUM.H ====---- */

#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "MATHCODE\plane3.h"
#include "collision.h"

/*------------------
---- STRUCTURES ----
------------------*/


class Frustum {
	private:
		Vector3		op[5];		// Original frustum points, 6th point is the center of sphere
		Vector3		wp[5];		// World points updated with calcWorldPoints
		Vector3		lookDir;	// Normalized view direction
		
		Plane3		side[6];	// Frustum planes

		BSphere		oSphere;	// Original collision sphere

		BSphere		wSphere;	// Sphere updated to world space with GetWorldPoints
		
		float		distToCorner;	// distance from frustum origin to a far corner

	public:
		const Vector3 &	getWorldPoint(int p) const { msgAssert(p >= 0 && p < 6, "frustum point out of range");
													 return wp[p]; }
		const BSphere &	getWorldSphere() const { return wSphere; }
		const Plane3  &	getSides() const { return side[0]; }
		const Vector3 &	getLookDir() const { return lookDir; }
		float			getDistToCorner() const { return distToCorner; }

		colTestType		sphereInsidePlanes(const BSphere &_sphere) const;

		bool			checkPointIn(const Vector3 &p) const;
		void			calcWorldPoints(const Vector3 &p, int zangle, int xangle, int yangle);
		void			draw(void) const;

		void			resetFrustum(float angle, float viewDist);

		explicit Frustum() {}
		explicit Frustum(float angle, float viewDist);
//		explicit Frustum(const Vector3 &o, const Vector3 &ul, const Vector3 &ll, const Vector3 &lr, const Vector3 &ur);
		~Frustum() {}
};

#endif