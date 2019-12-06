//	----==== MATRIX4X4.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	This is a 4x4 matrix stored in row-major format, so 4d vectors are
//					represented as a row when multiplying
//----------------------------------------------------------------------------------------

#ifndef MATRIX4X4_H
#define MATRIX4X4_H

/*------------------
---- STRUCTURES ----
------------------*/

class Vector4;


class Matrix4x4 {

	public:

		///// Variables

		float				i[16];

		///// Overloaded Operators

		__inline void		operator= (const Matrix4x4 &m);
		__inline void		operator*=(const Matrix4x4 &m);
		void				operator*=(const Vector4 &v);	// post multiply a column vector

		__inline void		multiply(const Matrix4x4 &m1, const Matrix4x4 &m2);
		__inline void		setIdentity(void);
		__inline void		setTranslation(float x, float y, float z);
		__inline void		setScaling(float x, float y, float z);
		__inline void		translate(float x, float y, float z);
		__inline void		scale(float x, float y, float z);
		
		///// Functions

		void				set(const Matrix4x4 &m) { for (int c = 0; c < 16; c++) i[c] = m.i[c]; }
		void				set(const Vector4 &r1, const Vector4 &r2, const Vector4 &r3, const Vector4 &r4);

		// floating point angles, uses trig calculations, SLOW
		void				setRotation(float x, float y, float z);
		void				rotateX(float a);	// these functions should not be concatenated, in other
		void				rotateY(float a);	// words they should only be used to quickly set a matrix
		void				rotateZ(float a);	// that is already known to be the identity matrix
		
		// integer angles used with LookupManager, uses trig tables, FAST
		void				setRotation(int x, int y, int z);
		void				rotateX(int a);		// same as above
		void				rotateY(int a);
		void				rotateZ(int a);

		// get transpose of matrix, usually used for giving to OpenGL
		Matrix4x4			getTranspose(void) const;
		__inline void		setTranspose(void);

		// Constructors / Destructor
		Matrix4x4() {}
		Matrix4x4(const Matrix4x4 &m) { for (int c = 0; c < 16; c++) i[c] = m.i[c]; }
		explicit			Matrix4x4(const Vector4 &r1, const Vector4 &r2, const Vector4 &r3, const Vector4 &r4);
		explicit			Matrix4x4(const float *p, int l);

		~Matrix4x4() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/


__inline void Matrix4x4::operator= (const Matrix4x4 &m)
{
	for (int c = 0; c < 16; c++) i[c] = m.i[c];
}


__inline void Matrix4x4::operator*=(const Matrix4x4 &m)
{
	Matrix4x4 orig(*this);

	// row 1
	i[0]  = orig.i[0]*m.i[0] + orig.i[1]*m.i[4] + orig.i[2]*m.i[8]  + orig.i[3]*m.i[12];
	i[1]  = orig.i[0]*m.i[1] + orig.i[1]*m.i[5] + orig.i[2]*m.i[9]  + orig.i[3]*m.i[13];
	i[2]  = orig.i[0]*m.i[2] + orig.i[1]*m.i[6] + orig.i[2]*m.i[10] + orig.i[3]*m.i[14];
	i[3]  = orig.i[0]*m.i[3] + orig.i[1]*m.i[7] + orig.i[2]*m.i[11] + orig.i[3]*m.i[15];

	// row 2
	i[4]  = orig.i[4]*m.i[0] + orig.i[5]*m.i[4] + orig.i[6]*m.i[8]  + orig.i[7]*m.i[12];
	i[5]  = orig.i[4]*m.i[1] + orig.i[5]*m.i[5] + orig.i[6]*m.i[9]  + orig.i[7]*m.i[13];
	i[6]  = orig.i[4]*m.i[2] + orig.i[5]*m.i[6] + orig.i[6]*m.i[10] + orig.i[7]*m.i[14];
	i[7]  = orig.i[4]*m.i[3] + orig.i[5]*m.i[7] + orig.i[6]*m.i[11] + orig.i[7]*m.i[15];

	// row 3
	i[8]  = orig.i[8]*m.i[0] + orig.i[9]*m.i[4] + orig.i[10]*m.i[8]  + orig.i[11]*m.i[12];
	i[9]  = orig.i[8]*m.i[1] + orig.i[9]*m.i[5] + orig.i[10]*m.i[9]  + orig.i[11]*m.i[13];
	i[10] = orig.i[8]*m.i[2] + orig.i[9]*m.i[6] + orig.i[10]*m.i[10] + orig.i[11]*m.i[14];
	i[11] = orig.i[8]*m.i[3] + orig.i[9]*m.i[7] + orig.i[10]*m.i[11] + orig.i[11]*m.i[15];

	// row 4
	i[12] = orig.i[12]*m.i[0] + orig.i[13]*m.i[4] + orig.i[14]*m.i[8]  + orig.i[15]*m.i[12];
	i[13] = orig.i[12]*m.i[1] + orig.i[13]*m.i[5] + orig.i[14]*m.i[9]  + orig.i[15]*m.i[13];
	i[14] = orig.i[12]*m.i[2] + orig.i[13]*m.i[6] + orig.i[14]*m.i[10] + orig.i[15]*m.i[14];
	i[15] = orig.i[12]*m.i[3] + orig.i[13]*m.i[7] + orig.i[14]*m.i[11] + orig.i[15]*m.i[15];
}


__inline void Matrix4x4::multiply(const Matrix4x4 &m1, const Matrix4x4 &m2)
{
	// row 1
	i[0]  = m1.i[0]*m2.i[0] + m1.i[1]*m2.i[4] + m1.i[2]*m2.i[8]  + m1.i[3]*m2.i[12];
	i[1]  = m1.i[0]*m2.i[1] + m1.i[1]*m2.i[5] + m1.i[2]*m2.i[9]  + m1.i[3]*m2.i[13];
	i[2]  = m1.i[0]*m2.i[2] + m1.i[1]*m2.i[6] + m1.i[2]*m2.i[10] + m1.i[3]*m2.i[14];
	i[3]  = m1.i[0]*m2.i[3] + m1.i[1]*m2.i[7] + m1.i[2]*m2.i[11] + m1.i[3]*m2.i[15];

	// row 2
	i[4]  = m1.i[4]*m2.i[0] + m1.i[5]*m2.i[4] + m1.i[6]*m2.i[8]  + m1.i[7]*m2.i[12];
	i[5]  = m1.i[4]*m2.i[1] + m1.i[5]*m2.i[5] + m1.i[6]*m2.i[9]  + m1.i[7]*m2.i[13];
	i[6]  = m1.i[4]*m2.i[2] + m1.i[5]*m2.i[6] + m1.i[6]*m2.i[10] + m1.i[7]*m2.i[14];
	i[7]  = m1.i[4]*m2.i[3] + m1.i[5]*m2.i[7] + m1.i[6]*m2.i[11] + m1.i[7]*m2.i[15];

	// row 3
	i[8]  = m1.i[8]*m2.i[0] + m1.i[9]*m2.i[4] + m1.i[10]*m2.i[8]  + m1.i[11]*m2.i[12];
	i[9]  = m1.i[8]*m2.i[1] + m1.i[9]*m2.i[5] + m1.i[10]*m2.i[9]  + m1.i[11]*m2.i[13];
	i[10] = m1.i[8]*m2.i[2] + m1.i[9]*m2.i[6] + m1.i[10]*m2.i[10] + m1.i[11]*m2.i[14];
	i[11] = m1.i[8]*m2.i[3] + m1.i[9]*m2.i[7] + m1.i[10]*m2.i[11] + m1.i[11]*m2.i[15];

	// row 4
	i[12] = m1.i[12]*m2.i[0] + m1.i[13]*m2.i[4] + m1.i[14]*m2.i[8]  + m1.i[15]*m2.i[12];
	i[13] = m1.i[12]*m2.i[1] + m1.i[13]*m2.i[5] + m1.i[14]*m2.i[9]  + m1.i[15]*m2.i[13];
	i[14] = m1.i[12]*m2.i[2] + m1.i[13]*m2.i[6] + m1.i[14]*m2.i[10] + m1.i[15]*m2.i[14];
	i[15] = m1.i[12]*m2.i[3] + m1.i[13]*m2.i[7] + m1.i[14]*m2.i[11] + m1.i[15]*m2.i[15];
}


// Set matrix to the identity matrix
__inline void Matrix4x4::setIdentity(void)
{
	i[0]  = 1; i[1]  = 0; i[2]  = 0; i[3]  = 0;
	i[4]  = 0; i[5]  = 1; i[6]  = 0; i[7]  = 0;
	i[8]  = 0; i[9]  = 0; i[10] = 1; i[11] = 0;
	i[12] = 0; i[13] = 0; i[14] = 0; i[15] = 1;
}


// Set matrix to a translation matrix
__inline void Matrix4x4::setTranslation(float x, float y, float z)
{
	i[0]  = 1; i[1]  = 0; i[2]  = 0; i[3]  = 0;
	i[4]  = 0; i[5]  = 1; i[6]  = 0; i[7]  = 0;
	i[8]  = 0; i[9]  = 0; i[10] = 1; i[11] = 0;
	i[12] = x; i[13] = y; i[14] = z; i[15] = 1;
}


// Set matrix to a scaling matrix
__inline void Matrix4x4::setScaling(float x, float y, float z)
{
	i[0]  = x; i[1]  = 0; i[2]  = 0; i[3]  = 0;
	i[4]  = 0; i[5]  = y; i[6]  = 0; i[7]  = 0;
	i[8]  = 0; i[9]  = 0; i[10] = z; i[11] = 0;
	i[12] = 0; i[13] = 0; i[14] = 0; i[15] = 1;
}


// Set translation portion of matrix, do not touch rest. This can be called after any other
// set function, but the translations are not additive
__inline void Matrix4x4::translate(float x, float y, float z)
{
	i[12] = x;
	i[13] = y;
	i[14] = z;
}


// Set scaling portion of matrix, do not touch rest
__inline void Matrix4x4::scale(float x, float y, float z)
{
	i[0] = x;
	i[5] = y;
	i[10] = z;
}


// set current matrix to its transpose
__inline void Matrix4x4::setTranspose(void)
{
	float temp = i[1];
	i[1] = i[4];
	i[4] = temp;

	temp = i[2];
	i[2] = i[8];
	i[8] = temp;

	temp = i[6];
	i[6] = i[9];
	i[9] = temp;

	temp = i[3];
	i[3] = i[12];
	i[12] = temp;

	temp = i[7];
	i[7] = i[13];
	i[13] = temp;

	temp = i[11];
	i[11] = i[14];
	i[14] = temp;
}


#endif