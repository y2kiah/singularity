/* ----==== WORLD_CONSOLE.CPP ====---- */

#include "world_console.h"
#include "world.h"
#include "engine.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Level_CAccess //////////

void Level_CAccess::levelUnload(void) const
{
	if (!level.unloadLevel()) console.addLine("     no level loaded");
}


void Level_CAccess::levelLoad(const char *_filename) const
{
	if (level.levelLoaded) level.unloadLevel();

	console.addLine("");
	if (level.loadLevel(std::string("data/level/")+ _filename)) {
		console.addLine(Color3f(1,1,1),"level \"%s\" loaded successfully", level.levelName.c_str());

	} else {
		level.unloadLevel();
		console.addLineErr("level \"%s\" loading unsuccessful",_filename);
	}
}


void Level_CAccess::levelName(void) const
{
	if (level.levelLoaded)
		console.addLine("     level name: %s",level.levelName.c_str());
	else
		console.addLine("     no level loaded");
}


void Level_CAccess::levelCreateHorizonMap(int texSize, const char *_filename) const
{
	level.getTerrain().createHorizonMap(texSize, _filename);
}


void Level_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"levelunload") == 0)	levelUnload();
	else if (_stricmp(_callname,"levelload") == 0)		levelLoad(_varlist[0]->s->c_str());
	else if (_stricmp(_callname,"levelname") == 0)		levelName();
	else if (_stricmp(_callname,"levelcreatehorizonmap") == 0)	levelCreateHorizonMap(_varlist[0]->i, _varlist[1]->s->c_str());
}


Level_CAccess::Level_CAccess()
{
	console.registerCommand("levelunload","()",this,0);
	console.registerCommand("levelload","(string levelFile)",this,1,CVAR_STRING);
	console.registerCommand("levelname","()",this,0);
	console.registerCommand("levelcreatehorizonmap","(int texSize, string filename)",this,2,CVAR_INT,CVAR_STRING);
}


Level_CAccess::~Level_CAccess()
{
	console.unregisterCommand("levelunload");
	console.unregisterCommand("levelload");
	console.unregisterCommand("levelname");
	console.unregisterCommand("levelcreatehorizonmap");
}