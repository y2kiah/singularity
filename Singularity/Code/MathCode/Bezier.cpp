//	----==== BEZIER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	A wrapper class for functions to calculate points on a Bezier
//					curve, and Bezier surface
//	--------------------------------------------------------------------------------

#include "bezier.h"
#include "vector3.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class BezierCurve //////////


Vector3 BezierCurve::calcPointOnCurve(float t, const Vector3 &p0, const Vector3 &p1,
									  const Vector3 &p2, const Vector3 &p3)
{
	// B(t) = P1 * (1 - t)^3 + P2 * 3 * t * (1 - t)^2 + P3 * 3 * t^2 * (1 - t) + P4 * t^3 

	// Store (1 - t)
	float var1 = 1 - t;
	
	// Store (1 - t)^3
	float var2 = var1 * var1 * var1;

	// Store t^3
	float var3 = t * t * t;

	// Store two coefficients into the following equations for speed
	float c1 = 3 * t * var1 * var1;
	float c2 = 3 * t * t * var1;

	return	Vector3(var2*p0.x + c1*p1.x + c2*p2.x + var3*p3.x,
					var2*p0.y + c1*p1.y + c2*p2.y + var3*p3.y,
					var2*p0.z + c1*p1.z + c2*p2.z + var3*p3.z);
}


Vector3 BezierCurve::calcPointOnPatch(float u, float v, Vector3 *pBuffer)
{
	Vector3 p0(calcPointOnCurve(u, pBuffer[0], pBuffer[1], pBuffer[2], pBuffer[3]));
	Vector3 p1(calcPointOnCurve(u, pBuffer[4], pBuffer[5], pBuffer[6], pBuffer[7]));
	Vector3 p2(calcPointOnCurve(u, pBuffer[8], pBuffer[9], pBuffer[10],pBuffer[11]));
	Vector3 p3(calcPointOnCurve(u, pBuffer[12],pBuffer[13],pBuffer[14],pBuffer[15]));

	return calcPointOnCurve(v, p0,p1,p2,p3);
}
