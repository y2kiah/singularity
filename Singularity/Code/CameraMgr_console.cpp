/* ----==== CAMERAMGR_CONSOLE.CPP ====---- */

#include "cameramgr_console.h"
#include "cameramanager.h"

#include "engine.h"	// remove when switch devmode bool to console variable

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class CameraMgr_CAccess //////////

void CameraMgr_CAccess::cameraSetActive(int set) const
{
	// if it's the devCam switch to dev mode
	if (set == camera.devCamID) devCam();
	else camera.setActiveID(set);
}


void CameraMgr_CAccess::cameraSetAngle(float a) const
{
	camera.camList[camera.activeCamID]->setAngle(a);
	console.addLine("     camera lens angle set to %0.2f", a);
}


void CameraMgr_CAccess::cameraSetViewDist(float d) const
{
	camera.camList[camera.activeCamID]->setViewDist(d);
	console.addLine("     camera view distance set to %0.2f", d);
}


void CameraMgr_CAccess::cameraZoom(float z) const
{
	float zoom = z;
	if (zoom < 0.5f) zoom = 0.5f;
	else if (zoom > 8.0f) zoom = 8.0f;
	camera.camList[camera.activeCamID]->setZoom(zoom);
}


void CameraMgr_CAccess::renderScene(bool r) const
{
	camera.camList[camera.activeCamID]->renderScene = r;
}


void CameraMgr_CAccess::devCam(void) const
{
	// set beginning position to current cam's position
//	Vector3 setP = engine.camList->getActiveCam().getP();
//	int xa = engine.camList->getActiveCam().getXAngle();
//	int ya = engine.camList->getActiveCam().getYAngle();
//	int za = engine.camList->getActiveCam().getZAngle();

	// switch to dev camera
	camera.setActiveID(camera.devCamID);
//	camera.getActiveCam().setPos(setP);
//	camera.getActiveCam().setOrientation(xa,ya,za);
	engine.cOptions.cameraView = true;
}


void CameraMgr_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"camerasetactive")==0)		cameraSetActive(_varlist[0]->i);
	else if (_stricmp(_callname,"camerasetangle")==0)		cameraSetAngle(_varlist[0]->f);
	else if (_stricmp(_callname,"camerasetviewdist")==0)	cameraSetViewDist(_varlist[0]->f);
	else if (_stricmp(_callname,"camerazoom")==0)			cameraZoom(_varlist[0]->f);
	else if (_stricmp(_callname,"renderscene")==0)			renderScene(_varlist[0]->b);
	else if (_stricmp(_callname,"devcam")==0)				devCam();
}


CameraMgr_CAccess::CameraMgr_CAccess()
{
	console.registerCommand("camerasetactive","(int camID)",this,1,CVAR_INT);
	console.registerCommand("camerasetangle","(float angle)",this,1,CVAR_FLOAT);
	console.registerCommand("camerasetviewdist","(float dist)",this,1,CVAR_FLOAT);
	console.registerCommand("camerazoom","(float zoom)",this,1,CVAR_FLOAT);
	console.registerCommand("renderscene","(bool render)",this,1,CVAR_BOOL);
	console.registerCommand("devcam","()",this,0);
}


CameraMgr_CAccess::~CameraMgr_CAccess()
{
	console.unregisterCommand("camerasetactive");
	console.unregisterCommand("camerasetangle");
	console.unregisterCommand("camerasetviewdist");
	console.unregisterCommand("camerazoom");
	console.unregisterCommand("renderscene");
	console.unregisterCommand("devcam");
}