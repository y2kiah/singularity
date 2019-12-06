/* ----==== MESHMGR_CONSOLE.H ====---- */

#ifndef MESHMGR_CONSOLE_H
#define MESHMGR_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class MeshMgr_CAccess : public CHandler {
	private:
		void meshList(void) const;
		void meshLoad(const char *_filename, bool swapYZ) const;
		void meshTexIDs(int meshID) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit MeshMgr_CAccess();
		~MeshMgr_CAccess();
};

#endif