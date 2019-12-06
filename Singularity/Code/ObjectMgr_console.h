/* ----==== OBJECTMGR_CONSOLE.H ====---- */

#ifndef OBJECTMGR_CONSOLE_H
#define OBJECTMGR_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class ObjectMgr_CAccess : public CHandler {
	private:
		void objectTypeList(void) const;
		void objectList(void) const;
		void placeObject(int _t) const;
		void createObjectType(const char *_name, int _meshID, bool _hwLight) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit ObjectMgr_CAccess();
		~ObjectMgr_CAccess();
};

#endif