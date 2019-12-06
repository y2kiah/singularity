/* ----==== PLAYER_CONSOLE.CPP ====---- */

#include "player_console.h"
#include "player.h"
#include "engine.h"
#include "cameramanager.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Player_CAccess //////////

void Player_CAccess::playerName(const char *_name) const
{
	strncpy_s(engine.player->name,_name,19);
	console.addLine("     player name: %s",engine.player->name);
}


void Player_CAccess::playerSpeed(const float s) const
{
	engine.player->maxspeed = s;
	console.addLine("     speed set to: %0.2f ft/sec",s);
}


void Player_CAccess::playerDrop(const float x,const float z) const
{
	engine.player->viewp.x = x;
	engine.player->p.x = engine.player->viewp.x;
	engine.player->viewp.z = z;
	engine.player->p.z = engine.player->viewp.z;
	engine.player->yangle = engine.player->xangle = engine.player->zangle = 0;
	console.addLine("     player dropped at: %0.2f, %0.2f",x,z);
}


void Player_CAccess::playerCam(void) const
{
	camera.setActiveID(engine.player->getViewCamID());
	engine.cOptions.cameraView = false;
}


void Player_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"playername") == 0)		playerName(_varlist[0]->s->c_str());
	else if (_stricmp(_callname,"playerspeed") == 0)	playerSpeed(_varlist[0]->f);
	else if (_stricmp(_callname,"playerdrop") == 0)		playerDrop(_varlist[0]->f,_varlist[1]->f);
	else if (_stricmp(_callname,"playercam") == 0)		playerCam();
}


Player_CAccess::Player_CAccess()
{
	console.registerCommand("playername","(string name)",this,1,CVAR_STRING);
	console.registerCommand("playerspeed","(float ftPerSecond)",this,1,CVAR_FLOAT);
	console.registerCommand("playerdrop","(float x, float z)",this,2,CVAR_FLOAT,CVAR_FLOAT);
	console.registerCommand("playercam","()",this,0);
}


Player_CAccess::~Player_CAccess()
{
	console.unregisterCommand("playername");
	console.unregisterCommand("playerspeed");
	console.unregisterCommand("playerdrop");
	console.unregisterCommand("playercam");
}