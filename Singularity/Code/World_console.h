/* ----==== WORLD_CONSOLE.H ====---- */

#ifndef WORLD_CONSOLE_H
#define WORLD_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Level_CAccess : public CHandler {
	private:
		void levelLoad(const char *_filename) const;
		void levelUnload(void) const;
		void levelName(void) const;
		void levelCreateHorizonMap(int texSize, const char *_filename) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit Level_CAccess();
		~Level_CAccess();
};

#endif