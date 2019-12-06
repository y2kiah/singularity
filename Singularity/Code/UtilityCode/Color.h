//	----==== COLOR.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Data types that store color information
//----------------------------------------------------------------------------------------


#ifndef COLOR_H
#define COLOR_H

/*------------------
---- STRUCTURES ----
------------------*/


class Color3f {
	public:
		union {
			struct {
				float r, g, b;
			};
			float v[3];
		};

		void operator=(const Color3f &c) { r = c.r; g = c.g; b = c.b; }

		void set(float _r,float _g,float _b) { r=_r; g=_g; b=_b; }

		Color3f(const Color3f &c) : r(c.r), g(c.g), b(c.b) {}
		Color3f(const float _r,const float _g,const float _b) : r(_r), g(_g), b(_b) {}
		explicit Color3f() : r(0), g(0), b(0) {}
};

class Color4f {
	public:
		union {
			struct {
				float r, g, b, a;
			};
			float v[4];
		};

		void operator=(const Color4f &c) { r = c.r; g = c.g; b = c.b; a = c.a; }
		void operator=(const Color3f &c) { r = c.r; g = c.g; b = c.b; }

		void set(float _r,float _g,float _b,float _a) { r=_r; g=_g; b=_b; a=_a; }

		Color4f(const Color4f &c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		Color4f(const float _r,const float _g,const float _b,const float _a) : r(_r), g(_g), b(_b), a(_a) {}
		explicit Color4f() : r(0), g(0), b(0), a(1) {}
};

class Color3uc {
	public:
		union {
			struct {
				unsigned char r, g, b;
			};
			unsigned char v[3];
		};

		void operator=(const Color3uc &c) { r = c.r; g = c.g; b = c.b; }

		void set(unsigned char _r,unsigned char _g,unsigned char _b) { r=_r; g=_g; b=_b; }

		Color3uc(const Color3uc &c) : r(c.r), g(c.g), b(c.b) {}
		Color3uc(unsigned char _r,unsigned char _g,unsigned char _b) : r(_r), g(_g), b(_b) {}
		explicit Color3uc() : r(0), g(0), b(0) {}
};

class Color4uc {
	public:
		union {
			struct {
				unsigned char r, g, b, a;
			};
			unsigned char v[4];
		};

		void operator=(const Color4uc &c) { r = c.r; g = c.g; b = c.b; a = c.a; }
		void operator=(const Color3uc &c) { r = c.r; g = c.g; b = c.b; }

		void set(unsigned char _r,unsigned char _g,unsigned char _b,unsigned char _a) { r=_r; g=_g; b=_b; a=_a; }

		Color4uc(const Color4uc &c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
		Color4uc(unsigned char _r,unsigned char _g,unsigned char _b,unsigned char _a) : r(_r), g(_g), b(_b), a(_a) {}
		explicit Color4uc() : r(0), g(0), b(0), a(255) {}
};


#endif