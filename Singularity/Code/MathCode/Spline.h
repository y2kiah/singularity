//	----==== SPLINE.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	A wrapper class for functions to calculate points on a Catmull-
//					Rom spline, as well as a bicubic surface
//	--------------------------------------------------------------------------------

#ifndef SPLINE_H
#define SPLINE_H

#include "matrix4x4.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Vector2;
class Vector3;
class Vector4;


class CatmullRomSpline {

	private:

		///// Used in precalc stage for heightfield mesh equations
		static Matrix4x4	basisMatrix;	// stores equation basis values
		static Matrix4x4	preCalcMatrix;	// stores precalc values of 4 splines in a patch
		static Matrix4x4	splineMatrix;	// used for finding point in a patch

		static void			preCalcCatmullRom(Vector4 &v, float p1, float p2, float p3, float p4);

	public:
		
		///// Complex mesh equations, SLOW
		
		static Vector3		calcPointOnSpline(float t, const Vector3 &p0, const Vector3 &p1,
											const Vector3 &p2, const Vector3 &p3);

		static Vector3		calcPointOnPatch(float u, float v, Vector3 *pBuffer);

		///// Heightfield mesh equations, FAST

		static void			setSplineMatrix(int xi, int zi, float *height, int size);
		static float		calcQuad(float u, float v);
//		static float		calcHeightInPatch(float u, float v);
};


class CubicBSpline {

	private:

		const static float	oneSixth;

		///// Used in precalc stage
		static Matrix4x4	basisMatrix;			// stores equation basis values
		static Matrix4x4	basisMatrixT;			// stores transpose of basis matrix
		static Matrix4x4	pointsMatrix;			// stores control points of patch
		static Matrix4x4	middleMatrix;			// stores precalc values
		static Matrix4x4 const *ptrMiddleMatrix;	// pointer to a middle matrix, prevents having to copy values
		static float		hSpacing;				// used to correct surface normals for scaled x,z spacing of surface maps
		static float		vSpacing;				// used to correct surface normals for scaled y spacing of surface maps
		static float		invVSpacing;			// for optimized concavity calculation
		
		static Vector4		preCalcVector;			// control points * basis
		static Vector3		preCalcTangentVector;
		static Vector2		preCalcConcavityVector;

	public:

		///// Heightfield non-matrix form, SLOW

		static float		calcHeightOnCubicBSpline(float t, float h0, float h1, float h2, float h3);
		static float		calcTangentOnCubicBSpline(float t, float h0, float h1, float h2, float h3);
		static float		calcConcavityOnCubicBSpline(float t, float h0, float h1, float h2, float h3);
		static float		calcHeightOnPatch(float u, float v, const float *hBuffer);

		///// Heightfield non-matrix form Pre-calc version, FASTER

		static Vector4		preCalcSpline(const Vector4 &pts);
		static Vector3		preCalcTangent(const Vector4 &pts);
		static Vector2		preCalcConcavity(const Vector4 &pts);
		static float		getPreCalcHeight(float t);
		static float		getPreCalcTangent(float t);
		static float		getPreCalcConcavity(float t);

		///// Heightfield matrix form, FASTEST

		static void			preCalcMiddleMatrix(const float *hBuffer);
		static void			setMiddleMatrix(const Matrix4x4 &m) { middleMatrix.set(m); }
		static void			setMiddleMatrixPtr(const Matrix4x4 &m) { ptrMiddleMatrix = &m; }
		static void			setSpacing(float h, float v) { hSpacing = h; vSpacing = v; invVSpacing = (vSpacing == 0) ? 0 : 1.0f / vSpacing; }
		static float		calcHeightOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix);
		static Vector3		calcNormalOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix);
		static float		calcConcavityOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix);
		static const Matrix4x4 & getMiddleMatrix() { return middleMatrix; }
};


class BSpline {

	private:

		///// Used with recursive algorithm
		static float		N(float t, int i, int k, float *knot);

	public:

		enum BSplineType {
			BSPLINE_TYPE_OPEN_NORMALIZED = 0,
			BSPLINE_TYPE_OPEN_NOT_NORMALIZED,
			BSPLINE_TYPE_PERIODIC_NORMALIZED,
			BSPLINE_TYPE_PERIODIC_NOT_NORMALIZED
		};

		static Vector3		calcPointOnBSpline(float t, int k, Vector3 *pts, int numPts, BSplineType splineType);

		static Vector3		calcPointOnCubicBSpline(float t, Vector3 *pts, int numPts, BSplineType splineType);
		
		static Vector3		calcPointOnBiCubicPatch(float u, float v, Vector3 *pBuffer,
												int bufferWidth, int bufferHeight, BSplineType splineType);
};


#endif