/* ----==== PLAYER_CONSOLE.H ====---- */

#ifndef PLAYER_CONSOLE_H
#define PLAYER_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Player_CAccess : public CHandler {
	private:
		void playerName(const char *_name) const;
		void playerSpeed(const float s) const;
		void playerDrop(const float x,const float z) const;
		void playerCam(void) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit Player_CAccess();
		~Player_CAccess();
};

#endif