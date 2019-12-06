/* ----==== OBJECTMGR_CONSOLE.CPP ====---- */

#include <direct.h>
#include "objectmanager.h"
#include "objectmgr_console.h"
#include "object.h"
#include "meshmanager.h"
#include "engine.h"
#include "player.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class ObjectMgr_CAccess //////////

void ObjectMgr_CAccess::objectTypeList(void) const
{
	if (objectMgr.typeListIsEmpty()) {
		console.addLine("     list empty");
	} else {
		for (int o = 0; o < objectMgr.objTypeList.size(); ++o) {
			console.addLine("     ID %i: %s", o, ".");//objectMgr.objTypeList[o]->toString().c_str());
		}
	}
}


void ObjectMgr_CAccess::objectList(void) const
{
	if (objectMgr.instanceListIsEmpty()) {
		console.addLine("     list empty");
	} else {
		for (int o = 0; o < objectMgr.objInstanceList.size(); ++o) {
			console.addLine("     ID %i: \"%s\": x=%0.2f, y=%0.2f, z=%0.2f", o,
				objectMgr.objTypeList[objectMgr.objInstanceList[o]->objTypeID]->objName.c_str(),
				objectMgr.objInstanceList[o]->worldPos.x, objectMgr.objInstanceList[o]->worldPos.y, objectMgr.objInstanceList[o]->worldPos.z);
		}
	}
}


void ObjectMgr_CAccess::placeObject(int _t) const
{
	if (_t < 0 || _t >= objectMgr.objTypeList.size()) {
		console.addLineErr("     object ID out of range");
		return;
	} else {		
		Object_Instance *newObjInstance = new Object_Instance(_t);
		newObjInstance->setWorldPos(engine.player->p,0,0,0);
		objectMgr.pushInstance(newObjInstance);
		console.addLine("     object \"%s\" created at x=%0.2f, y=%0.2f, z=%0.2f",
			objectMgr.objTypeList[_t]->objName.c_str(), engine.player->p.x, engine.player->p.y, engine.player->p.z);
	}
}


void ObjectMgr_CAccess::createObjectType(const char *_name, int _meshID, bool _hwLight) const
{
	if (_meshID < 0 || _meshID >= mesh.meshListSize()) {
		console.addLine("     mesh ID out of range");
		return;
	} else {
		Object_Type *newObjType = new Object_Type(_name, _meshID);
		newObjType->doHWLight = _hwLight;
		objectMgr.pushType(newObjType);
		console.addLine("     object type \"%s\" created", _name);
	}
}


void ObjectMgr_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"objecttypelist")==0) objectTypeList();
	else if	(_stricmp(_callname,"objectlist")==0) objectList();
	else if	(_stricmp(_callname,"placeobject")==0) placeObject(_varlist[0]->i);
	else if	(_stricmp(_callname,"createobjecttype")==0) createObjectType(_varlist[0]->s->c_str(), _varlist[1]->i, _varlist[2]->b);
}


ObjectMgr_CAccess::ObjectMgr_CAccess()
{
	console.registerCommand("objecttypelist","()",this,0);
	console.registerCommand("objectlist","()",this,0);
	console.registerCommand("placeobject","(int objectTypeID)",this,1,CVAR_INT);
	console.registerCommand("createobjecttype","(string name, int meshID, bool hwLighting)",this,3,CVAR_STRING,CVAR_INT,CVAR_BOOL);
}


ObjectMgr_CAccess::~ObjectMgr_CAccess()
{
	console.unregisterCommand("objecttypelist");
	console.unregisterCommand("objectlist");
	console.unregisterCommand("placeobject");
	console.unregisterCommand("createobjecttype");
}