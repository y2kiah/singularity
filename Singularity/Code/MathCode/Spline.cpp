//	----==== SPLINE.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	A wrapper class for functions to calculate points on a Catmull-
//					Rom spline, as well as a class to store a spline
//	--------------------------------------------------------------------------------


#include "spline.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

/*----------------------
---- STATIC MEMBERS ----
----------------------*/

////////// class CatmullRomSpline //////////

Matrix4x4 CatmullRomSpline::basisMatrix(Vector4( 0.0f,  1.0f,  0.0f,  0.0f),
										Vector4(-0.5f,  0.0f,  0.5f,  0.0f),
										Vector4( 1.0f, -2.5f,  2.0f, -0.5f),
										Vector4(-0.5f,  1.5f, -1.5f,  0.5f));

Matrix4x4 CatmullRomSpline::preCalcMatrix;
Matrix4x4 CatmullRomSpline::splineMatrix;


////////// class CubicBSpline //////////

// x(t)  = (h0)(1/6)(1-3t+3t^2-t^3) + (h1)(1/6)(4-6t^2+3t^3) + (h2)(1/6)(1+3t+3t^2-3t^3) + (h3)(1/6)(t^3)
Matrix4x4 CubicBSpline::basisMatrix(Vector4( 1.0f, 4.0f, 1.0f, 0.0f)/6.0f,
									Vector4(-3.0f,    0, 3.0f, 0.0f)/6.0f,
									Vector4( 3.0f,-6.0f, 3.0f, 0.0f)/6.0f,
									Vector4(-1.0f, 3.0f,-3.0f, 1.0f)/6.0f);

Matrix4x4 CubicBSpline::basisMatrixT(basisMatrix.getTranspose());

const float CubicBSpline::oneSixth = 1.0f / 6.0f;
Matrix4x4 CubicBSpline::pointsMatrix;
Matrix4x4 CubicBSpline::middleMatrix;
Matrix4x4 const * CubicBSpline::ptrMiddleMatrix = 0;
float CubicBSpline::hSpacing = 1.0f;
float CubicBSpline::vSpacing = 1.0f;
float CubicBSpline::invVSpacing = 1.0f;

Vector4 CubicBSpline::preCalcVector(0,0,0,0);
Vector3 CubicBSpline::preCalcTangentVector(0,0,0);
Vector2 CubicBSpline::preCalcConcavityVector(0,0);

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class CatmullRomSpline //////////


////////////////////////////////////////////////////////////////////////////////////////////////////
//	calcPointOnSpline
//
//		Calculates a point on a 2d Catmull-Rom Spline, defined by four control points. The resulting
//		point is determined by normalized time t spanning segment p1<->p2, so the point will fall
//		somewhere between control points p1 and p2. One point is required before and after the
//		segment, making the total 4.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 CatmullRomSpline::calcPointOnSpline(float t, const Vector3 &p0, const Vector3 &p1,
											const Vector3 &p2, const Vector3 &p3)
{
	const float t2 = t*t;
	const float t3 = t2*t;
	
	float returnX = 0.5f * ((2*p1.x) + (-p0.x + p2.x) * t + 
		(2*p0.x - 5*p1.x + 4*p2.x - p3.x) * t2 + (-p0.x + 3*p1.x - 3*p2.x + p3.x) * t3);

	float returnY = 0.5f * ((2*p1.y) + (-p0.y + p2.y) * t + 
		(2*p0.y - 5*p1.y + 4*p2.y - p3.y) * t2 + (-p0.y + 3*p1.y - 3*p2.y + p3.y) * t3);

	float returnZ = 0.5f * ((2*p1.z) + (-p0.z + p2.z) * t + 
		(2*p0.z - 5*p1.z + 4*p2.z - p3.z) * t2 + (-p0.z + 3*p1.z - 3*p2.z + p3.z) * t3);

	return Vector3(returnX,returnY,returnZ);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	calcPointOnPatch
//
//		Calculates a point on a 3d bicubic patch, made up of 16 control points for splines passing
//		through the patch. Point is calculated by taking points at time u from 4 splines, and taking
//		point at time v of resulting spline. Resulting point will be within the quad of p5,p6,p9,p10
//		seen as the middle quad of the 16 points.
//		
//		Equation:
//			f(v, f(u,P0,P1,P2,P3), f(u,P4,P5,P6,P7), f(u,P8,P9,P10,P11), f(u,P12,P13,P14,P15))
//
////////////////////////////////////////////////////////////////////////////////////////////////////
Vector3 CatmullRomSpline::calcPointOnPatch(float u, float v, Vector3 *pBuffer)
{
	Vector3 p0(calcPointOnSpline(u, pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]));
	Vector3 p1(calcPointOnSpline(u, pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]));
	Vector3 p2(calcPointOnSpline(u, pBuffer[8], pBuffer[9], pBuffer[10],pBuffer[11]));
	Vector3 p3(calcPointOnSpline(u, pBuffer[12],pBuffer[13],pBuffer[14],pBuffer[15]));

	return calcPointOnSpline(v, p0,p1,p2,p3);
}


void CatmullRomSpline::preCalcCatmullRom(Vector4 &v, float p1, float p2, float p3, float p4)
{
    v.x = p2;
    v.y = (p3 - p1) * 0.5f;
    v.z = p1 - 2.5f*p2 + 2.0f*p3 - 0.5f*p4;
    v.w = -0.5f*p1 + 1.5f*p2 - 1.5f*p3 + 0.5f*p4;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	setSplineMatrix
//
//		This function sets splineMatrix according to the index values given into a height field
//		of floats.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void CatmullRomSpline::setSplineMatrix(int xi, int zi, float *height, int size)
{
	int ri = 0;

	for (int i = 0; i < 4; i++) {
		Vector4 pMatV;

		int index = (zi+i)*size + xi;
		preCalcCatmullRom(pMatV, height[index], height[index+1], height[index+2], height[index+3]);

		preCalcMatrix.i[ri]   = pMatV.x;
		preCalcMatrix.i[ri+1] = pMatV.y;
		preCalcMatrix.i[ri+2] = pMatV.z;
		preCalcMatrix.i[ri+3] = pMatV.w;

		ri += 4;
	}

	splineMatrix = basisMatrix;
	splineMatrix *= preCalcMatrix;
}


float CatmullRomSpline::calcQuad(float u, float v)
{
    Vector4 v1(1.0f, u, u*u, u*u*u);
	Vector4 v2(1.0f, v, v*v, v*v*v);
    
	// may need to be v1 here
	v2 *= splineMatrix;
    
	return v1 * v2;
}


/*float CatmullRomSpline::calcHeightInPatch(float x, float z)
{
	// temp //
	float quadScaleY = 1.0f, quadBaseY = 0;
	//////////

	assert(x >= 0 && x <= 1 && z >= 0 && z <= 1);

    // calculate and return quad height
    return quadScaleY * calcQuad(x,z) + quadBaseY;
}*/


////////// class CubicBSpline //////////

float CubicBSpline::calcHeightOnCubicBSpline(float t, float h0, float h1, float h2, float h3)
{
	// x(t)  = (h0)(1/6)(1-3t+3t^2-t^3) + (h1)(1/6)(4-6t^2+3t^3) + (h2)(1/6)(1+3t+3t^2-3t^3) + (h3)(1/6)(t^3)

	const float oneOverSix = oneSixth;
	const float t2 = t * t;
	const float t3 = t2 * t;

	return	(h0 * oneOverSix * (1 - 3*t + 3*t2 - t3)) +
			(h1 * oneOverSix * (4 - 6*t2 + 3*t3)) +
			(h2 * oneOverSix * (1 + 3*t + 3*t2 - 3*t3)) +
			(h3 * oneOverSix * t3);
}


float CubicBSpline::calcTangentOnCubicBSpline(float t, float h0, float h1, float h2, float h3)
{
	// x'(t) = (h0)(1/6)(-3+6t-3t^2) + (h1)(1/6)(-12t+9t^2) + (h2)(1/6)(3+6t-9t^2) + (h3)(1/6)(3t^2)

	const float oneOverSix = oneSixth;
	const float t2 = t * t;

	return	(h0 * oneOverSix * (-3 + 6*t - 3*t2)) +
			(h1 * oneOverSix * (-12*t + 9*t2)) +
			(h2 * oneOverSix * (3 + 6*t - 9*t2)) +
			(h3 * oneOverSix * (3*t2));
}


float CubicBSpline::calcConcavityOnCubicBSpline(float t, float h0, float h1, float h2, float h3)
{
	// x''(t) = (h0)(1/6)(6-6t) + (h1)(1/6)(-12+18t) + (h2)(1/6)(6-18t) + (h3)(1/6)(6t)

	const float oneOverSix = oneSixth;

	return	(h0 * oneOverSix * (6 - 6*t)) +
			(h1 * oneOverSix * (-12 + 18*t)) +
			(h2 * oneOverSix * (6 - 18*t)) +
			(h3 * oneOverSix * (6*t));
}


float CubicBSpline::calcHeightOnPatch(float u, float v, const float *hBuffer)
{
	float p0,p1,p2,p3;
	
	// if u is within range of 1.0, flip spline and use 0
	if (fabs(u - 1.0f) <= EPSILON) {
		u = 0;

		p0 = calcHeightOnCubicBSpline(u, hBuffer[3], hBuffer[2], hBuffer[1], hBuffer[0]);
		p1 = calcHeightOnCubicBSpline(u, hBuffer[7], hBuffer[6], hBuffer[5], hBuffer[4]);
		p2 = calcHeightOnCubicBSpline(u, hBuffer[11],hBuffer[10],hBuffer[9], hBuffer[8]);
		p3 = calcHeightOnCubicBSpline(u, hBuffer[15],hBuffer[14],hBuffer[13],hBuffer[12]);

	} else {

		p0 = calcHeightOnCubicBSpline(u, hBuffer[0], hBuffer[1], hBuffer[2], hBuffer[3]);
		p1 = calcHeightOnCubicBSpline(u, hBuffer[4], hBuffer[5], hBuffer[6], hBuffer[7]);
		p2 = calcHeightOnCubicBSpline(u, hBuffer[8], hBuffer[9], hBuffer[10],hBuffer[11]);
		p3 = calcHeightOnCubicBSpline(u, hBuffer[12],hBuffer[13],hBuffer[14],hBuffer[15]);

	}

	// if v is within range of 1.0, flip spline and use 0
	if (fabs(v - 1.0f) <= EPSILON) {
		v = 0;

		return calcHeightOnCubicBSpline(v, p3,p2,p1,p0);
	}

	return calcHeightOnCubicBSpline(v, p0,p1,p2,p3);
}


///// Pre-calc functions
Vector4 CubicBSpline::preCalcSpline(const Vector4 &pts)
{
//	preCalcVector = pts * basisMatrix;

//	SAME THING AS ABOVE BUT FASTER
//	p1 + 4p2 + p3
//	-3p1 + 3p3
//	3p1 - 6p2 + 3p3
//	-p1 + 3p2 - 3p3 + p4

	preCalcVector.x = pts.x + 4*pts.y + pts.z;
	preCalcVector.y = -3*pts.x + 3*pts.z;
	preCalcVector.z = 3*pts.x - 6*pts.y + 3*pts.z;
	preCalcVector.w = -pts.x + 3*pts.y - 3*pts.z + pts.w;

	preCalcVector *= oneSixth;

	return preCalcVector;
}


Vector3 CubicBSpline::preCalcTangent(const Vector4 &pts)
{
//	preCalcTangentVector = pts * basisMatrixDerivative;

//	SAME THING AS ABOVE BUT FASTER
//	-3p1 + 3p3
//	6p1 - 12p2 + 6p3
//	-3p1 + 9p2 - 9p3 + 3p4

	preCalcTangentVector.x = -3*pts.x + 3*pts.z;
	preCalcTangentVector.y = 6*pts.x - 12*pts.y + 6*pts.z;
	preCalcTangentVector.z = -3*pts.x + 9*pts.y - 9*pts.z + 3*pts.w;

	preCalcTangentVector *= oneSixth;

	return preCalcTangentVector;
}


Vector2 CubicBSpline::preCalcConcavity(const Vector4 &pts)
{
//	preCalcTangentVector = pts * basisMatrixDerivative;

//	SAME THING AS ABOVE BUT FASTER
//	6p1 - 12p2 + 6p3
//	-6p1 + 18p2 - 18p3 + 6p4

	preCalcConcavityVector.x = 6*pts.x - 12*pts.y + 6*pts.z;
	preCalcConcavityVector.y = -6*pts.x + 18*pts.y - 18*pts.z + 6*pts.w;

	preCalcConcavityVector *= oneSixth;

	return preCalcConcavityVector;
}


float CubicBSpline::getPreCalcHeight(float t)
{
	Vector4 tVec(1.0f, t, t*t, t*t*t);

	return tVec * preCalcVector;
}


float CubicBSpline::getPreCalcTangent(float t)
{
	Vector3 tVec(1.0f, t, t*t);
	
	return tVec * preCalcTangentVector;
}


float CubicBSpline::getPreCalcConcavity(float t)
{
	Vector2 tVec(1.0f, t);
	
	return tVec * preCalcConcavityVector;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	preCalcMiddleMatrix
//
//		Expects a float buffer of 16 values (11,12,13,14,21,22,23,24,31,32,33,34,41,42,43,44)
//		and sets the middle matrix for the patch
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void CubicBSpline::preCalcMiddleMatrix(const float *hBuffer)
{
	for (int c = 0; c < 16; c++) pointsMatrix.i[c] = hBuffer[c];

	middleMatrix = basisMatrix;
	middleMatrix *= pointsMatrix;
	middleMatrix *= basisMatrixT;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	calcHeightOnPatchMatrix
//
//		Finds the height on a patch for a certain (u,v). Must call preCalcMiddleMatrix before using
//		this function for a certain patch
//
////////////////////////////////////////////////////////////////////////////////////////////////////
float CubicBSpline::calcHeightOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix)
{
	Vector4 v1(1.0f, u, u*u, u*u*u);
	Vector4 v2(1.0f, v, v*v, v*v*v);

	if (usePtrMiddleMatrix) {
		v2 *= (*ptrMiddleMatrix);
	} else {
		v2 *= middleMatrix;
	}
    
	return v1 * v2;
}


Vector3	CubicBSpline::calcNormalOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix)
{
	// calc derivative with respect to v
	Vector4 vec1(1.0f, u, u*u, u*u*u);
	Vector4 vec2(0, 1, 2*v, 3*v*v);

	if (usePtrMiddleMatrix) {
		vec2 *= (*ptrMiddleMatrix);
	} else {
		vec2 *= middleMatrix;
	}
	Vector3 nv(0, (vec1 * vec2)*vSpacing, hSpacing);

	// calc derivative with respect to u
	vec1.assign(0, 1, 2*u, 3*u*u);
	vec2.assign(1.0f, v, v*v, v*v*v);

	if (usePtrMiddleMatrix) {
		vec2 *= (*ptrMiddleMatrix);
	} else {
		vec2 *= middleMatrix;
	}
	Vector3 nu(hSpacing, (vec1 * vec2)*vSpacing, 0);

	// cross product gives normal
	Vector3 n;
	n.unitNormalOf(nv, nu);
	
	return n;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	calcConcavityOnPatchMatrix
//
//		Returns the summation of the absolute value of the vertical distances between the sample
//		control point and the two adjacent control points. This is when the concavity is sampled at
//		the control point (t=0 or t=1.0)
//
////////////////////////////////////////////////////////////////////////////////////////////////////
float CubicBSpline::calcConcavityOnPatchMatrix(float u, float v, bool usePtrMiddleMatrix)
{
	// calc second derivative with respect to v
	Vector4 vec1(1.0f, u, u*u, u*u*u);
	Vector4 vec2(0, 0, 2.0f, 6*v);

	if (usePtrMiddleMatrix) {
		vec2 *= (*ptrMiddleMatrix);
	} else {
		vec2 *= middleMatrix;
	}
	float first = fabs((vec1 * vec2) * invVSpacing);

	// calc second derivative with respect to u
	vec1.assign(0, 0, 2.0f, 6*u);
	vec2.assign(1.0f, v, v*v, v*v*v);

	if (usePtrMiddleMatrix) {
		vec2 *= (*ptrMiddleMatrix);
	} else {
		vec2 *= middleMatrix;
	}
	float second = fabs((vec1 * vec2) * invVSpacing);

	//return (second > first) ? second : first;
	return first + second;
}


////////// class BSpline //////////


float BSpline::N(float t, int i, int k, float *knot)
{
	if (k == 1) {
		if (knot[i] <= t && t < knot[i+1]) return 1.0f;
		return 0;
	}

	float term1 = 0;
	float term2 = 0;

	if (knot[i+k-1] - knot[i] != 0)
		term1 = (t - knot[i]) * N(t, i, k-1, knot) / (knot[i+k-1] - knot[i]);

	if (knot[i+k] - knot[i+1] != 0)
		term2 = (knot[i+k] - t) * N(t, i+1, k-1, knot) / (knot[i+k] - knot[i+1]);

	return term1 + term2;
}


Vector3 BSpline::calcPointOnBSpline(float t, int k, Vector3 *pts, int numPts, BSplineType splineType)
{
	msgAssert(k >= 2 && k <= numPts, "BSpline::calcPointOnBSpline k out of range");

	Vector3 returnVector(0,0,0);

	// N+k elements == (numPts-1)+k
	const int n = numPts - 1;
	const int segments = n - k + 2;
	const int knotElements = numPts - 1 + k;

	float *knot = new float[knotElements];

	if (splineType == BSPLINE_TYPE_OPEN_NORMALIZED) {

		// Make an open/uniform knot vector (normalized)
		float knotStep = 1.0f / segments;
		float knotVal = 0;
		for (int c = 0; c < knotElements; c++) {			
			knot[c] = knotVal;
			if (c >= k-1 && c < numPts) knotVal += knotStep;
		}

	} else if (splineType == BSPLINE_TYPE_OPEN_NOT_NORMALIZED) {

		// Make an open/uniform knot vector
		int knotVal = 0;
		for (int c = 0; c < knotElements; c++) {
			knot[c] = (float)knotVal;
			if (c >= k-1 && c < numPts) knotVal++;
		}

	} else if (splineType == BSPLINE_TYPE_PERIODIC_NORMALIZED) {

		// Make a periodic/uniform knot vector (normalized)
		float knotStep = 1.0f / segments;
		float knotVal = 0;
		for (int s = 0; s < knotElements; s++) {
			knot[s] = knotVal;
			knotVal += knotStep;
		}

	} else if (splineType == BSPLINE_TYPE_PERIODIC_NOT_NORMALIZED) {

		// Make a periodic/uniform knot vector (not normalized)
		int knotVal = 0;
		for (int s = 0; s < knotElements; s++) {
			knot[s] = (float)knotVal;
			knotVal++;
		}

	}

	// Find point on the spline
	for (int i = 0; i < numPts; i++)
		returnVector += pts[i] * N(t, i, k, knot);

	delete [] knot;

	return returnVector;
}


Vector3 BSpline::calcPointOnCubicBSpline(float t, Vector3 *pts, int numPts, BSplineType splineType)
{
	Vector3 returnVector(0,0,0);

	// N+k elements == (numPts-1)+k
	float *knot = new float[numPts+3];

	if (splineType == BSPLINE_TYPE_OPEN_NORMALIZED) {

		// Make an open/uniform knot vector (normalized)
		float knotStep = 1.0f / (numPts-3);
		float knotVal = 0;
		for (int c = 0; c < numPts+3; c++) {
			knot[c] = knotVal;
			if (c >= 3 && c < numPts) knotVal += knotStep;
		}

	} else if (splineType == BSPLINE_TYPE_OPEN_NOT_NORMALIZED) {

		// Make an open/uniform knot vector
		int knotVal = 0;
		for (int c = 0; c < numPts+3; c++) {
			knot[c] = (float)knotVal;
			if (c >= 3 && c < numPts) knotVal++;
		}

	} else if (splineType == BSPLINE_TYPE_PERIODIC_NORMALIZED) {

		// Make a periodic/uniform knot vector (normalized)
		float knotStep = 1.0f / (numPts+4);
		float knotVal = 0;
		for (int s = 0; s < numPts+3; s++) {
			knot[s] = knotVal;
			knotVal += knotStep;
		}

	} else if (splineType == BSPLINE_TYPE_PERIODIC_NOT_NORMALIZED) {

		// Make a periodic/uniform knot vector (not normalized)
		int knotVal = 0;
		for (int s = 0; s < numPts+3; s++) {
			knot[s] = (float)knotVal;
			knotVal++;
		}

	}

	// Find point on the spline
	for (int i = 0; i < numPts; i++)
		returnVector += pts[i] * N(t, i, 4, knot);

	delete [] knot;

	return returnVector;
}


Vector3 BSpline::calcPointOnBiCubicPatch(float u, float v, Vector3 *pBuffer,
								  int bufferWidth, int bufferHeight, BSplineType splineType)
{
	Vector3 *p = new Vector3[bufferHeight];

	Vector3 *bufferPosition = pBuffer;
	for (int h = 0; h < bufferHeight; h++) {
		p[h] = calcPointOnCubicBSpline(u, bufferPosition, bufferWidth, splineType);

		bufferPosition += bufferWidth;
	}

	return calcPointOnCubicBSpline(v, p, bufferHeight, splineType);
}