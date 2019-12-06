//	----==== KEYBOARDMANAGER.CPP ====----
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

#include "keyboardmanager.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class KeyboardManager //////////


//------------------------------------------------------------------------------
//	The following two functions convert scan or virtual codes to Ascii charaters
//	corrected for keyboard layout and character sets. The character returned can
//	be obtained by casting the result arguement to a char. Returns 0 on error,
//	1 for one char set in result or 2 for two chars set in result. Only a return
//	of 1 should be considered valid.
//------------------------------------------------------------------------------
int KeyboardManager::vkToAscii(UINT vkCode, unsigned short * result)
{		
	DWORD scanCode = MapVirtualKeyEx(vkCode, 0, layout);

	return ToAsciiEx(vkCode, scanCode, state, result, 0, layout);
}


int KeyboardManager::scanToAscii(DWORD scanCode, unsigned short * result)
{		
	UINT vkCode = MapVirtualKeyEx(scanCode, 1, layout);

	return ToAsciiEx(vkCode, scanCode, state, result, 0, layout);
}


//------------------------------------------------------------------------------
//	This function checks all useful keys for typed input and appends the
//	characters to inputString. This function should only be called once per
//	frame and MUST be called before ALL other calls to keyDown, keyTyped, or
//	buttonPressed. If calls to these functions are made before this one in the
//	game loop, some keys will not register. If possible try not to mix calls to
//	keyDown and keyTyped or this within the same loop iteration for the same key
//------------------------------------------------------------------------------
bool KeyboardManager::appendAllInput(std::string & inputString)
{
	if (GetKeyboardState(state) == 0) return false;

	for (int k = 0x20; k < 256; ++k) {
		if (keyTyped(k)) {
			unsigned short character;

			// If the function did not copy 1 character to the buffer then don't add the result
			// Either there was an error (0) or 2 characters were returned (2)
			if (vkToAscii(k,&character) != 1) return false;
			
			inputString += (char)character;
		}
	}

	return true;
}


KeyboardManager::KeyboardManager() : Singleton<KeyboardManager>(*this)
{ 
	layout = GetKeyboardLayout(0);
	memset(state,0,256);
	memset(buttonDown,0,256);
}

