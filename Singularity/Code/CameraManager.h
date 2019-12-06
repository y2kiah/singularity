/* ----==== CAMERAMANAGER.H ====---- */

#ifndef CAMERAMANAGER_H
#define CAMERAMANAGER_H

#include <vector>
#include "camera.h"
#include "UTILITYCODE\singleton.h"

/*---------------
---- DEFINES ----
---------------*/

#define camera		CameraManager::instance()	// used to access the CameraManager instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class Frustum;
class CHandler;
class CameraMgr_CAccess;


class CameraManager : public Singleton<CameraManager> {
	friend class CameraMgr_CAccess;

	private:
		std::vector<Camera*>	camList;

		int						activeCamID;
		int						devCamID;

		CHandler				*cHandler;

	public:
		
		int		createNewCamera(const Vector3 &p, int xa, int ya, int za, float va, float vd, bool fog);
//		void	removeID(int camID);
		void	clear(void);

		void	setActiveID(int camID);
		void	renderActive(void) const;

		int		getActiveID() const { return activeCamID; }

		Camera &	getActiveCam(void) const	{ return *camList[activeCamID]; }
		Camera &	getCamera(int camID) const	{ msgAssert(camID >= 0 && camID < camList.size(), "camera out of range");													
												  return *camList[camID]; }

		explicit CameraManager();
		~CameraManager();
};

#endif