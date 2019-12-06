/* ----==== SKYBOX_CONSOLE.CPP ====---- */

#include <stdio.h>
#include "skybox_console.h"
#include "skybox.h"
#include "world.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Skybox_CAccess //////////

void Skybox_CAccess::layerSpeed(int _layer, float _us, float _vs) const
{
	if (_layer < 0 || _layer >= level.skyBox->layer.size()) {
		console.addLine("     layer %i out of range", _layer);
	} else {
		if (level.skyBox->layer[_layer]->isActive()) {
			level.skyBox->layer[_layer]->setSpeed(_us, _vs);
			console.addLine("     layer %i speed changed to x: %0.4f, z: %0.4f",_layer,_us,_vs);
		} else {
			console.addLine("     layer %i not active", _layer);
		}
	}
}


void Skybox_CAccess::layerActivate(int _layer, float _us, float _vs) const
{
	if (_layer < 0 || _layer >= level.skyBox->layer.size()) {
		console.addLine("     layer %i out of range", _layer);
	} else {
		if (level.skyBox->layer[_layer]->isActive()) {
			console.addLine("     layer %i is already active", _layer);			
		} else {
			level.skyBox->layer[_layer]->setActive(true);
			level.skyBox->layer[_layer]->setSpeed(_us, _vs);
			console.addLine("     layer %i activated with speed x: %0.4f, z: %0.4f", _layer,_us,_vs);
		}
	}
}


void Skybox_CAccess::layerDeactivate(int _layer) const
{
	if (_layer < 0 || _layer >= level.skyBox->layer.size()) {
		console.addLine("     layer %i out of range", _layer);
	} else {
		if (!level.skyBox->layer[_layer]->isActive()) {
			console.addLine("     layer %i is not active", _layer);			
		} else {
			level.skyBox->layer[_layer]->setActive(false);			
			console.addLine("     layer %i deactivated", _layer);
		}
	}
}


void Skybox_CAccess::layerActiveList(void) const
{
	for (int c = 0; c < level.skyBox->layer.size(); ++c) {
		if (level.skyBox->layer[c]->isActive()) {
			console.addLine("     layer %i, active, speed x: %0.4f, z: %0.4f, texID: %i", c, level.skyBox->layer[c]->getUSpeed(), level.skyBox->layer[c]->getVSpeed(), level.skyBox->layer[c]->getTexID());
		} else {
			console.addLine("     layer %i, inactive, speed x: %0.4f, z: %0.4f, texID: %i", c, level.skyBox->layer[c]->getUSpeed(), level.skyBox->layer[c]->getVSpeed(), level.skyBox->layer[c]->getTexID());
		}
	}
}


void Skybox_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"layerspeed") == 0)			layerSpeed(_varlist[0]->i,_varlist[1]->f,_varlist[2]->f);
	else if (_stricmp(_callname,"layeractivate") == 0)		layerActivate(_varlist[0]->i,_varlist[1]->f,_varlist[2]->f);
	else if (_stricmp(_callname,"layerdeactivate") == 0)	layerDeactivate(_varlist[0]->i);
	else if (_stricmp(_callname,"layeractivelist") == 0)	layerActiveList();
}


Skybox_CAccess::Skybox_CAccess()
{
	console.registerCommand("layerspeed","(int layerID, float uSpeed, float vSpeed)",this,3,CVAR_INT,CVAR_FLOAT,CVAR_FLOAT);
	console.registerCommand("layeractivate","(int layerID, float uSpeed, float vSpeed)",this,3,CVAR_INT,CVAR_FLOAT,CVAR_FLOAT);
	console.registerCommand("layerdeactivate","(int layerID)",this,1,CVAR_INT);
	console.registerCommand("layeractivelist","()",this,0);
}


Skybox_CAccess::~Skybox_CAccess()
{
	console.unregisterCommand("layerspeed");
	console.unregisterCommand("layeractivate");
	console.unregisterCommand("layerdeactivate");
	console.unregisterCommand("layeractivelist");
}