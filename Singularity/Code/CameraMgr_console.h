/* ----==== CAMERAMGR_CONSOLE.H ====---- */

#ifndef CAMERAMGR_CONSOLE_H
#define CAMERAMGR_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class CameraMgr_CAccess : public CHandler {
	private:
		void cameraSetActive(int set) const;
		void cameraSetAngle(float a) const;
		void cameraSetViewDist(float d) const;
		void cameraZoom(float z) const;
		void renderScene(bool r) const;
		void devCam(void) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit CameraMgr_CAccess();
		~CameraMgr_CAccess();
};

#endif