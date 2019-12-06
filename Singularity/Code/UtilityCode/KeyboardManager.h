//	----==== KEYBOARDMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	This class is a wrapper for the Win32 keyboard functions.
//					It handles fast checking of key positions, input typing, and
//					toggle key states. Ascii conversion is provided which
//					corrects for different keyboard layouts and character sets.
//	----------------------------------------------------------------------------


#ifndef KEYBOARDMANAGER_H
#define KEYBOARDMANAGER_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <string>
#include "singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define kbManager	KeyboardManager::instance()		// used to access the KeyboardManager instance globally
#define kb			KeyboardManager::instance()


/*------------------
---- STRUCTURES ----
------------------*/

class KeyboardManager : public Singleton<KeyboardManager> {
	private:

		///// Variables

		HKL				layout;
		unsigned char	state[256];
		unsigned char	buttonDown[256];

	public:

		///// Functions

		//------------------------------------------------------------------------------
		//	The keyDown function is the fastest way to get the state of an individual
		//	key using standard Win32 calls. Keep in mind that although this is ideal for
		//	games because it is faster, the input is UNBUFFERED, so it is possible for
		//	some keypresses to be missed if the framerate gets too low.
		//------------------------------------------------------------------------------
//		bool	keyDown(UINT vkCode) const { return (GetAsyncKeyState(vkCode) & 0x8000) ? true : false; }
		bool	keyDown(UINT vkCode) const { return (GetAsyncKeyState(vkCode) != 0) ? true : false; }
		
		//------------------------------------------------------------------------------
		//	keyTyped should be used when keyboard input is needed in a typing fashion,
		//	that is, holding the key will put a character up initially, followed by a
		//	stream of characters after some delay. The appendAllInput function should be
		//	called instead of this whenever possible.
		//------------------------------------------------------------------------------		
		bool	keyTyped(UINT vkCode) const { return (GetAsyncKeyState(vkCode) & 0x0001) ? true : false; }
//		bool	keyTyped(UINT vkCode) const { return (GetKeyState(vkCode) & 1) ? true : false; }

		//------------------------------------------------------------------------------
		//	keyToggled determines whether the key is toggled on or off. Useful keys for
		//	this function are Caps Lock or Num Lock, however any key may be toggled.
		//------------------------------------------------------------------------------
		bool	keyToggled(UINT vkCode) const { return (GetKeyState(vkCode) & 1) ? true : false; }
		
		__inline bool	buttonPressed(UINT vkCode);

		// Misc

		int		vkToAscii(UINT vkCode, unsigned short * result);
		int		scanToAscii(DWORD scanCode, unsigned short * result);

		bool	appendAllInput(std::string &inputString);

		// Constructors / Destructor

		explicit KeyboardManager();
		~KeyboardManager() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/

////////// class KeyboardManager //////////


__inline bool KeyboardManager::buttonPressed(UINT vkCode)
{
	if (keyDown(vkCode)) {
		if (buttonDown[vkCode] != 1) {
			buttonDown[vkCode] = 1;
			return true;
		}
	} else buttonDown[vkCode] = 0;

	return false;
}

#endif