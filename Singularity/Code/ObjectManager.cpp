//	----==== OBJECTMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			3/06
//	Description:
//	----------------------------------------------------------------------------


#include "objectmanager.h"
#include "objectmgr_console.h"
#include "object.h"
#include "camera.h"
#include "UTILITYCODE\msgassert.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class ObjectManager //////////


void ObjectManager::renderObjects(const Camera &cam) const
{
	for (int o = 0; o < objInstanceList.size(); ++o) {
		objInstanceList[o]->render(cam);
	}
}


int ObjectManager::pushType(Object_Type *_objType)
{
	// Check if it's in the list already, if so delete the new mesh and return the current ID
	for (int i = 0; i < objTypeList.size(); ++i) {
		if (_stricmp(objTypeList[i]->getObjName().c_str(), _objType->getObjName().c_str()) == 0) {
			console.addLineErr("     object \"%s\" already loaded, using existing ID of %i", _objType->getObjName().c_str(), i);
			delete _objType;
			return i;
		}
	}

	// The mesh is not in the list so push it onto the front and return its ID
	objTypeList.push_back(_objType);
	return objTypeList.size() - 1;
}


void ObjectManager::clearTypes(void)
{
	for (int t = 0; t < objTypeList.size(); ++t) delete objTypeList[t];
	objTypeList.clear();
}


const Object_Type & ObjectManager::getObjectType(int o) const
{
	msgAssert(o >= 0 && o < objTypeList.size(), "object type out of range");
	return *objTypeList[o];		
}


int ObjectManager::pushInstance(Object_Instance *_objInst)
{
	objInstanceList.push_back(_objInst);
	return objInstanceList.size() - 1;
}


void ObjectManager::clearInstances(void)
{
	for (int i = 0; i < objInstanceList.size(); ++i) delete objInstanceList[i];
	objInstanceList.clear();
}


const Object_Instance & ObjectManager::getObjectInstance(int o) const
{
	msgAssert(o >= 0 && o < objInstanceList.size(), "object out of range");
	return *objInstanceList[o];	
}


ObjectManager::ObjectManager() : Singleton<ObjectManager>(*this)
{
	cHandler = new ObjectMgr_CAccess();
}


ObjectManager::~ObjectManager()
{
	clearTypes();
	clearInstances();
	delete cHandler;
}
