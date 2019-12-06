//	----==== GLFONT.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			1/04
//	Description:	Provides link between Windows TrueType fonts and bitmapped
//					fonts used by OpenGL. Will render text to an OpenGL window.
//	----------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL\glew.h>
#include <cstdio>
#include "glfont.h"

extern	HDC		hDC;

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class GLFont //////////

//---------------------------------------------------------------------
//	The function inverses the y coordinate so (0,0) will become top-
//	left of the screen instead of bottom-left as in OpenGL. The coords
//	specify the lower-left position of the text.
//---------------------------------------------------------------------
__inline void GLFont::positionText(int x, int y) const
{
	glPushAttrib(GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);

	glMatrixMode(GL_PROJECTION);						// Set our matrix to our projection matrix
	glPushMatrix();										// Push on a new matrix to work with
	glLoadIdentity();									// reset the matrix
	glMatrixMode(GL_MODELVIEW);							// Set our matrix to our model view matrix
	glPushMatrix();										// Push on a new matrix to work with
	glLoadIdentity();									// Reset that matrix

	y = resY - y;										// Calculate the weird screen position

	glViewport(x - 1, y - 1, 0, 0);						// Create a new viewport to draw into

	glRasterPos4f(0, 0, 0, 1);							// Set the drawing position

	glPopMatrix();										// Pop the current modelview matrix off the stack
	glMatrixMode(GL_PROJECTION);						// Go back into projection mode
	glPopMatrix();										// Pop the projection matrix off the stack

	glPopAttrib();										// This restores our TRANSFORM and VIEWPORT attributes
}


//---------------------------------------------------------------------
//	Returns false if the font fails to load
//---------------------------------------------------------------------
bool GLFont::loadTrueTypeFont(const char *font, int _size, int weight, DWORD italic, DWORD underline, DWORD strikeout)
{
	HFONT newFont;
	glFontID = glGenLists(96);

	newFont = CreateFont(	-_size,							// Height Of Font
							0,								// Width Of Font
							0,								// Angle Of Escapement
							0,								// Orientation Angle
							weight,							// Font Weight
							italic,							// Italic
							underline,						// Underline
							strikeout,						// Strikeout
							ANSI_CHARSET,					// Character Set Identifier
							OUT_TT_PRECIS,					// Output Precision
							CLIP_DEFAULT_PRECIS,			// Clipping Precision
							ANTIALIASED_QUALITY,			// Output Quality
							FF_DONTCARE | DEFAULT_PITCH,	// Family And Pitch
							font);							// Font Name

	if (newFont == NULL) return false;

	size = _size;

	SelectObject(hDC, newFont);
	wglUseFontBitmaps(hDC, 32, 96, glFontID);
	DeleteObject(newFont);

	fontCreated = true;

	return true;
}


//---------------------------------------------------------------------
//	Prints a string of characters on the screen. The function uses a
//	variable length arguement list to provide for printf formatting of
//	the string with variables.
//---------------------------------------------------------------------
void GLFont::print(const char *text, ...) const
{
	if (!fontCreated) return;

	char tempString[255] = "\0";
	va_list	arg;								// Pointer to list of arguments

	if (text == NULL) return;

	va_start(arg, text);						// Parses the string for variables
	vsprintf_s(tempString, text, arg);			// And converts symbols to actual numbers
	va_end(arg);								// Results are stored in text line

	glPushAttrib(GL_LIST_BIT);
	
	glListBase(glFontID - 32);
	glCallLists(strlen(tempString), GL_UNSIGNED_BYTE, tempString);

	glPopAttrib();
}


void GLFont::print(int x, int y, const char *text, ...) const
{
	if (!fontCreated) return;

	positionText(x, y);

	char tempString[255] = "\0";
	va_list	arg;								// Pointer to list of arguments

	if (text == NULL) return;

	va_start(arg, text);						// Parses the string for variables
	vsprintf_s(tempString, text, arg);			// And converts symbols to actual numbers
	va_end(arg);								// Results are stored in text line

	glPushAttrib(GL_LIST_BIT);
	
	glListBase(glFontID - 32);
	glCallLists(strlen(tempString), GL_UNSIGNED_BYTE, tempString);

	glPopAttrib();
}


//---------------------------------------------------------------------
//	The current screen resolution must be supplied to the font when it
//	is first created so it can be positioned on the screen correctly
//---------------------------------------------------------------------
GLFont::GLFont(int _resY)
{
	glFontID = 0;
	resY = _resY;
	size = 0;
	fontCreated = false;
}


GLFont::~GLFont()
{
	if (fontCreated) glDeleteLists(glFontID, 96);
}
