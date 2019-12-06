//	----==== BEZIER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	A wrapper class for functions to calculate points on a Bezier
//					curve, and Bezier surface
//	--------------------------------------------------------------------------------

#ifndef BEZIER_H
#define BEZIER_H

//#include "matrix4x4.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Vector3;
//class Vector4;


class BezierCurve {

	private:

//		static Matrix4x4	basisMatrix;	// stores equation basis values
//		static Matrix4x4	preCalcMatrix;	// stores precalc values of 4 splines in a patch
//		static Matrix4x4	splineMatrix;	// used for finding point in a patch

	public:
		
		///// Complex mesh equations, SLOW

		static Vector3	calcPointOnCurve(float t, const Vector3 &p0, const Vector3 &p1,
											const Vector3 &p2, const Vector3 &p3);

		static Vector3	calcPointOnPatch(float u, float v, Vector3 *pBuffer);

		///// Heightfield mesh equations, FAST

//		static void		preCalcCatmullRom(Vector4 &v, float p1, float p2, float p3, float p4);
//		static void		setSplineMatrix(int xi, int zi, float *height, int size);
//		static float	calcQuad(float u, float v);
//		static float	calcHeightInPatch(float u, float v);
};


#endif
