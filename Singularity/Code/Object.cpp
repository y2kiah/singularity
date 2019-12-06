/* ----==== OBJECT.CPP ====---- */

#include "object.h"
#include "objectmanager.h"
#include "meshmanager.h"
#include "camera.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Object_Type //////////


Object_Type::Object_Type(const char *_name, int _meshID)
{
	objName = _name;
	meshID = (_meshID >= 0) ? _meshID : 0;
	doHWLight = true;
}


////////// class Object_Instance //////////


void Object_Instance::render(const Camera &cam) const
{
	mesh.getMesh(objectMgr.getObjectType(objTypeID).getMeshID()).render(worldPos, xAngle, yAngle, zAngle, cam);
}


Object_Instance::Object_Instance(int _objType) : objTypeID(_objType)
{
	worldPos.assign(0,0,0);
	xAngle = yAngle = zAngle = 0;
}
