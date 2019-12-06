/* ----==== TEXTUREMGR_CONSOLE.H ====---- */

#ifndef TEXTUREMGR_CONSOLE_H
#define TEXTUREMGR_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class TextureMgr_CAccess : public CHandler {
	private:
		void textureList(void) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit TextureMgr_CAccess();
		~TextureMgr_CAccess();
};

#endif