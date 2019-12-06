//	----==== MATRIX4X4.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	This is a 4x4 matrix stored in row-major format, so 4d vectors are
//					represented as a row when multiplying
//----------------------------------------------------------------------------------------


#include <cmath>
#include "matrix4x4.h"
#include "vector3.h"
#include "vector4.h"
#include "..\UTILITYCODE\msgassert.h"
#include "..\UTILITYCODE\lookupmanager.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Matrix4x4 //////////

void Matrix4x4::operator *=(const Vector4 &v)
{
	i[0]  *= v.x; i[1]  *= v.y; i[2]  *= v.z; i[3]  *= v.w;
	i[4]  *= v.x; i[5]  *= v.y; i[6]  *= v.z; i[7]  *= v.w;
	i[8]  *= v.x; i[9]  *= v.y; i[10] *= v.z; i[11] *= v.w;
	i[12] *= v.x; i[13] *= v.y; i[14] *= v.z; i[15] *= v.w;
}

void Matrix4x4::set(const Vector4 &r1, const Vector4 &r2, const Vector4 &r3, const Vector4 &r4)
{
	i[0]  = r1.x; i[1]  = r1.y; i[2]  = r1.z; i[3]  = r1.w;
	i[4]  = r2.x; i[5]  = r2.y; i[6]  = r2.z; i[7]  = r2.w;
	i[8]  = r3.x; i[9]  = r3.y; i[10] = r3.z; i[11] = r3.w;
	i[12] = r4.x; i[13] = r4.y; i[14] = r4.z; i[15] = r4.w;
}


void Matrix4x4::setRotation(float h, float p, float b)
{
	float sh = sinf(h); float ch = cosf(h);
	float sp = sinf(p); float cp = cosf(p);
	float sb = sinf(b); float cb = cosf(b);
	
	i[0] = ch * cb + sh * sp * sb;
	i[1] = -ch * sb + sh * sp * cb;
	i[2] = sh * cp;

	i[4] = sb * cp;
	i[5] = cb * cp;
	i[6] = -sp;

	i[8] = -sh * cb + ch * sp * sb;
	i[9] = sb * sh + ch * sp * cb;
	i[10] = ch * cp;

	i[12] = i[13] = i[14] = 0;
}


void Matrix4x4::rotateX(float a)
{
	i[5] = cosf(a);
	i[6] = sinf(a);
	i[9] = -i[6];	
	i[10] = i[5];
}


void Matrix4x4::rotateY(float a)
{
	i[0] = cosf(a);
	i[8] = sinf(a);
	i[2] = -i[8];	
	i[10] = i[0];
}


void Matrix4x4::rotateZ(float a)
{
	i[0] = cosf(a);
	i[1] = sinf(a);
	i[4] = -i[1];	
	i[5] = i[0];
}


void Matrix4x4::setRotation(int h, int p, int b)
{
	msgAssert(h >= 0 && h < math.ANGLE360, "Matrix angle out of range");
	msgAssert(p >= 0 && p < math.ANGLE360, "Matrix angle out of range");
	msgAssert(b >= 0 && b < math.ANGLE360, "Matrix angle out of range");

	float sh = math.getSin(h); float ch = math.getCos(h);
	float sp = math.getSin(p); float cp = math.getCos(p);
	float sb = math.getSin(b); float cb = math.getCos(b);
	
	i[0] = ch * cb + sh * sp * sb;
	i[1] = -ch * sb + sh * sp * cb;
	i[2] = sh * cp;

	i[4] = sb * cp;
	i[5] = cb * cp;
	i[6] = -sp;

	i[8] = -sh * cb + ch * sp * sb;
	i[9] = sb * sh + ch * sp * cb;
	i[10] = ch * cp;

	i[12] = i[13] = i[14] = 0;
}


void Matrix4x4::rotateX(int a)
{
	msgAssert(a >= 0 && a < math.ANGLE360, "Matrix angle out of range");

	i[5] = math.getCos(a);
	i[6] = math.getSin(a);
	i[9] = -i[6];	
	i[10] = i[5];
}


void Matrix4x4::rotateY(int a)
{
	msgAssert(a >= 0 && a < math.ANGLE360, "Matrix angle out of range");

	i[0] = math.getCos(a);
	i[8] = math.getSin(a);
	i[2] = -i[8];	
	i[10] = i[0];
}


void Matrix4x4::rotateZ(int a)
{
	msgAssert(a >= 0 && a < math.ANGLE360, "Matrix angle out of range");

	i[0] = math.getCos(a);
	i[1] = math.getSin(a);
	i[4] = -i[1];	
	i[5] = i[0];
}


// get transpose of matrix, usually used for giving to OpenGL
Matrix4x4 Matrix4x4::getTranspose(void) const
{
	Vector4 r1(i[0], i[4], i[8],  i[12]);
	Vector4 r2(i[1], i[5], i[9],  i[13]);
	Vector4 r3(i[2], i[6], i[10], i[14]);
	Vector4 r4(i[3], i[7], i[11], i[15]);

	return Matrix4x4(r1, r2, r3, r4);
}


Matrix4x4::Matrix4x4(const Vector4 &r1, const Vector4 &r2, const Vector4 &r3, const Vector4 &r4)
{
	i[0]  = r1.x; i[1]  = r1.y; i[2]  = r1.z; i[3]  = r1.w;
	i[4]  = r2.x; i[5]  = r2.y; i[6]  = r2.z; i[7]  = r2.w;
	i[8]  = r3.x; i[9]  = r3.y; i[10] = r3.z; i[11] = r3.w;
	i[12] = r4.x; i[13] = r4.y; i[14] = r4.z; i[15] = r4.w;
}


Matrix4x4::Matrix4x4(const float *p, int l)
{
	msgAssert(p && (l > 0) && (l < 16), "Matrix constructor fail");

	for (int c = 0; c < l; c++) i[c] = p[c];
}