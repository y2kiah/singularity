//	----==== MESHMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:
//	----------------------------------------------------------------------------


#include "meshmanager.h"
#include "meshmgr_console.h"
#include "mesh.h"
#include "UTILITYCODE\msgassert.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class MeshManager //////////

int MeshManager::push(Mesh3D_Type *model)
{
	// Check if it's in the list already, if so delete the new mesh and return the current ID
	for (int i = 0; i < meshList.size(); ++i) {
		if (_stricmp(meshList[i]->getFilename().c_str(), model->getFilename().c_str()) == 0) {
			console.addLineErr("     mesh \"%s\" already loaded, using existing ID of %i", model->getFilename().c_str(), i);
			delete model;
			return i;
		}
	}

	// The mesh is not in the list so push it onto the front and return its ID
	meshList.push_back(model);
	return meshList.size() - 1;
}


void MeshManager::pop(void)
{
	delete meshList.back();
	meshList.pop_back();
}


void MeshManager::clear(void)
{
	for (int m = 0; m < meshList.size(); ++m) delete meshList[m];
	meshList.clear();
}


const Mesh3D_Type &MeshManager::getMesh(int m) const
{
	msgAssert(m >= 0 && m < meshList.size(), "mesh out of range");
	return *meshList[m];
}


MeshManager::MeshManager() : Singleton<MeshManager>(*this)
{
	cHandler = new MeshMgr_CAccess();
}


MeshManager::~MeshManager()
{
	clear();
	delete cHandler;
}
