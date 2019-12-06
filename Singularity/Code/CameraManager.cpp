/* ----==== CAMERAMANAGER.CPP ====---- */

#include "cameramanager.h"
#include "cameramgr_console.h"
#include "console.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class CameraManager //////////

// returns the ID of the added camera, -1 on error
int CameraManager::createNewCamera(const Vector3 &p, int xa, int ya, int za, float va, float vd, bool fog)
{
	camList.push_back(new Camera(p,xa,ya,za,va,vd,fog));

	return camList.size() - 1;
}


/*void CameraList::removeID(int camID)
{
	if (camID < 0 || camID >= camList.size() || camID == devCamID) {
		console.addLine(255,64,64,"unsuccessfully tried to remove camera ID %i",camID);
		return;
	}

	if (activeCamID == camID) activeCamID = devCamID;

	delete camList[camID];
	camList.erase(camList.begin() + camID);
}*/


void CameraManager::clear(void)
{
	for (int i = 0; i < camList.size(); i++) delete camList[i];
	camList.clear();
}


void CameraManager::setActiveID(int camID)
{
	if (camID < 0 || camID >= camList.size()) {
		console.addLine("     camera ID %i not found, active camera not changed", camID);
		return;
	}

	activeCamID = camID;
	camList[activeCamID]->show();

	console.addLine("     camera %i set as active", camID);
}


void CameraManager::renderActive(void) const
{
	camList[activeCamID]->renderWorld();
}


CameraManager::CameraManager() : Singleton<CameraManager>(*this)
{
	devCamID = createNewCamera(Vector3(0,0,0), 0, 0, 0, 60.0f, 10000.0f, false);
	setActiveID(devCamID);
	cHandler = new CameraMgr_CAccess;
}

CameraManager::~CameraManager()
{
	clear();
	delete cHandler;
}
