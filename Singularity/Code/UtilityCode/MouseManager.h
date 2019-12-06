//	----==== MOUSEMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		4
//	Date:			1/04
//	Description:	This class is a wrapper for the windows mouse. It handles
//					movement, button clicks, and the cursor. Want to switch to
//					DirectX for exclusive access when in RECENTER mode.
//	--------------------------------------------------------------------------------


#ifndef MOUSEMANAGER_H
#define MOUSEMANAGER_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <string>
#include "singleton.h"

/*---------------
---- DEFINES ----
---------------*/

#define mManager	MouseManager::instance()	// used to access the MouseManager instance globally
#define mouse		MouseManager::instance()


/*------------------
---- STRUCTURES ----
------------------*/


enum MouseMode {
	MMODE_NONE = 0,		// mouse mode is not defined
	MMODE_FREE,			// mouse cursor allowed to roam free, bound to screen
	MMODE_RECENTER		// mouse automatically recenters each frame
};


//------------------------------------------------------------------------------
//	MouseCursor is used by MouseManager to allow for custom cursors. As many
//	cursors as needed may be loaded. Switching between them while the engine is
//	running is very fast since all file loading is done during engine setup.
//------------------------------------------------------------------------------
class MouseCursor {
	private:

		bool			texturedCursor;
		std::string		textureFilename;

	public:
		

		explicit MouseCursor() : texturedCursor(false) {}
		~MouseCursor() {}
};


//------------------------------------------------------------------------------
//	MouseManager is a wrapper for the Win32 mouse functionality. Included are
//	methods to find the current position and distance that the mouse moved over
//	the last frame. The mouse can be operated in 2 different modes as of now.
//	Free mode allows the cursor to move freely and will be bound to the edges of
//	the monitor. It also sets the cursor to visible. Recenter mode is used for
//	mouse-look functionality in games, and causes the mouse to recenter each
//	frame. The deltaX and deltaY values will still be calculated for each frame,
//	but mouse position will be useless in this mode.
//------------------------------------------------------------------------------
class MouseManager : public Singleton<MouseManager> {
	private:

		///// Variables

		POINT			mousePos, oldPos;
		int				deltaX, deltaY;
		float			sensitivityMultiplier;
		
		bool			cursorVisible;		
		bool			leftClicked;		// these bools for checking single clicks
		bool			rightClicked;
		bool			middleClicked;

		int				resX, resY;			// screen resolution given to constructor

		MouseCursor *	mouseCursor;
		MouseMode		mouseMode;

	public:
		
		///// Functions

		// Accessors
		int			getX(void) const { return mousePos.x; }
		int			getY(void) const { return mousePos.y; }
		int			getDeltaX(void) const { return deltaX; }
		int			getDeltaY(void) const { return deltaY; }
		float		getSensitiveDeltaX(void) const { return sensitivityMultiplier * deltaX; }
		float		getSensitiveDeltaY(void) const { return sensitivityMultiplier * deltaY; }
		bool		getVisible(void) const { return cursorVisible; }
		MouseMode	getMode(void) const { return mouseMode; }

		int			getResX(void) const { return resX; }
		int			getResY(void) const { return resY; }

		// Mutators
		void			setVisible(bool s) { cursorVisible = s; ShowCursor(cursorVisible); }
		void			switchVisible(void) { cursorVisible = !cursorVisible; ShowCursor(cursorVisible); }
		void			switchMode(MouseMode m);
		__inline void	setSensitivity(float s);

		void			setResX(int _resX) { resX = abs(_resX); }
		void			setResY(int _resY) { resY = abs(_resY); }

		// Mouse Button Routines
		void	updateMousePosition(void);		// called once per frame
		
		// The following 3 check only if the button is down but will not handle situations
		// when the programmer wants to make sure the user released the button before
		// considering the next click. This is the fastest means of detecting a button state.
		bool	leftButtonDown(void) const { return (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? true : false; }
		bool	rightButtonDown(void) const { return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? true : false; }
		bool	middleButtonDown(void) const { return (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? true : false; }
		
		// These functions will make sure that the user has released the button
		// before considering it a new click. It takes buffered input from windows
		// messages unlike the 3 above
		__inline bool	leftButtonClicked(void);
		__inline bool	rightButtonClicked(void);
		__inline bool	middleButtonClicked(void);

		// Constructors / Destructor
		explicit MouseManager(int _resX, int _resY);
		~MouseManager() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/

// In C++, inline functions that are not written within the class definition must be written
// in the header file and not the .cpp file or they will not be recognized publicly

////////// class MouseManager //////////


__inline void MouseManager::setSensitivity(float s)
{
	// negative values are illegal
	if (s < 0) s = 1.0f;
	
	sensitivityMultiplier = s;
}


__inline bool MouseManager::leftButtonClicked(void)
{
	if (IsLButtonDown()) {
		if (!leftClicked) {
			leftClicked = true;
			return true;
		}
	} else leftClicked = false;
	
	return false;
}


__inline bool MouseManager::rightButtonClicked(void)
{
	if (IsRButtonDown()) {
		if (!rightClicked) {
			rightClicked = true;
			return true;
		}
	} else rightClicked = false;
	
	return false;
}


__inline bool MouseManager::middleButtonClicked(void)
{
	if (IsMButtonDown()) {
		if (!middleClicked) {
			middleClicked = true;
			return true;
		}
	} else middleClicked = false;
	
	return false;
}


#endif
