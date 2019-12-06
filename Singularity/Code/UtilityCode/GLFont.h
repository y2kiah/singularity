//	----==== GLFONT.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			1/04
//	Description:	Provides link between Windows TrueType fonts and bitmapped
//					fonts used by OpenGL. Will render text to an OpenGL window.
//	----------------------------------------------------------------------------

#ifndef GLFONT_H
#define GLFONT_H

typedef unsigned long	DWORD;

/*------------------
---- STRUCTURES ----
------------------*/

//------------------------------------------------------------------------------
//	Stores a font for use with OpenGL
//------------------------------------------------------------------------------
class GLFont {
	private:

		unsigned int	glFontID;
		int				resY;
		int				size;
		bool			fontCreated;
		
		__inline void positionText(int x, int y) const;

	public:
		
		void	print(const char *text, ...) const;
		void	print(int x, int y, const char *text, ...) const;
		bool	loadTrueTypeFont(const char *font, int _size, int weight, DWORD italic, DWORD underline, DWORD strikeout);
		int		getSize(void) const { return size; }

		explicit GLFont(int _resY);
		~GLFont();
};

#endif