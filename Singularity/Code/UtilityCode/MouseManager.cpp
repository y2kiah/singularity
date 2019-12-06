//	----==== MOUSEMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		4
//	Date:			1/04
//	Description:	This class is a wrapper for the windows mouse. It handles
//					movement, button clicks, and the cursor. Want to switch to
//					DirectX for exclusive access when in RECENTER mode.
//	----------------------------------------------------------------------------


#include "mousemanager.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class MouseCursor //////////


////////// class MouseManager //////////


//-------------------------------------------------------------------------
//	The mouse mode dictates the behavior of the mouse cursor and how
//	certain values are calculated each frame. To switch modes without
//	altering cursor visibility another call to setVisibility() must be made
//	after switchMode().
//-------------------------------------------------------------------------
void MouseManager::switchMode(MouseMode m)
{
	// If the mode is already set, don't try to set it again
	if (mouseMode == m) return;
	
	mouseMode = m;

	switch (mouseMode) {
		case MMODE_NONE:
		case MMODE_FREE:
			// recenter and make visible
			setVisible(TRUE);
			SetCursorPos(resX / 2, resY / 2);
			updateMousePosition();
			break;

		case MMODE_RECENTER:
			// recenter and make invisible
			setVisible(FALSE);
			SetCursorPos(resX / 2, resY / 2);
			updateMousePosition();
			break;

		default:
			// if mode is erroneous just set to MMODE_NONE
			switchMode(MMODE_NONE);			
	}
}


//-------------------------------------------------------------------------
//	This function must be called once per frame to update the member
//	attributes on the current state of the mouse. After calling this near
//	the start of each frame, retrieve the information with the accessors.
//-------------------------------------------------------------------------
void MouseManager::updateMousePosition(void)
{
	GetCursorPos(&mousePos);			// find new cursor position

	if (mouseMode == MMODE_RECENTER) {
		deltaX = mousePos.x - (resX / 2);
		deltaY = mousePos.y - (resY / 2);
		// if not free mode mouse should re-center so it never reaches an edge		
		SetCursorPos(resX / 2, resY / 2);

	} else {
		deltaX = mousePos.x - oldPos.x;		// find distance it traveled this frame
		deltaY = mousePos.y - oldPos.y;
		oldPos = mousePos;					// update the old position for next frame
	}
}


MouseManager::MouseManager(int _resX, int _resY) : Singleton<MouseManager>(*this)
{
	resX = abs(_resX);
	resY = abs(_resY);

	sensitivityMultiplier = 1.0f;

	switchMode(MMODE_NONE);

	leftClicked =
	rightClicked =
	middleClicked = false;

	mouseCursor = 0;
}