/* ----==== SHADERMGR_CONSOLE.H ====---- */

#ifndef SHADERMGR_CONSOLE_H
#define SHADERMGR_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class ShaderMgr_CAccess : public CHandler {
	private:
		void shaderList(void) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit ShaderMgr_CAccess();
		~ShaderMgr_CAccess();
};

#endif