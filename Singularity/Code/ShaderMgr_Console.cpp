/* ----==== SHADERMGR_CONSOLE.CPP ====---- */

#include <direct.h>
#include "shadermanager.h"
#include "shadermgr_console.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class ShaderMgr_CAccess //////////

void ShaderMgr_CAccess::shaderList(void) const
{
	if (shader.isEmpty()) {
		console.addLine("     list empty");
	} else {
		for (int s = 0; s < shader.programs.size(); ++s) {
			console.addLine("     ID %i: %s", s, shader.programs[s]->toString().c_str());
		}
	}
}

void ShaderMgr_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"shaderlist")==0) shaderList();
}


ShaderMgr_CAccess::ShaderMgr_CAccess()
{
	console.registerCommand("shaderlist","()",this,0);
}


ShaderMgr_CAccess::~ShaderMgr_CAccess()
{
	console.unregisterCommand("shaderlist");
}