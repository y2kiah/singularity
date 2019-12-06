/* ----==== MESHMGR_CONSOLE.CPP ====---- */

#include <direct.h>
#include "meshmanager.h"
#include "meshmgr_console.h"
#include "mesh.h"
#include "engine.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class MeshMgr_CAccess //////////

void MeshMgr_CAccess::meshList(void) const
{
	if (mesh.isEmpty()) {
		console.addLine("     list empty");
	} else {
		for (int m = 0; m < mesh.meshList.size(); ++m) {
			console.addLine("     ID %i: %s", m, mesh.meshList[m]->toString().c_str());
		}
	}
}

void MeshMgr_CAccess::meshLoad(const char *_filename, bool swapYZ) const
{
	_chdir("data/models/");

	Mesh3D_Type *newMesh = new Mesh3D_Type;

	if (!newMesh->load3DS(_filename, swapYZ)) {
		delete newMesh;
	} else {
		console.addLine("     loaded mesh \"%s\" successfully", _filename);
		mesh.push(newMesh);
	}

	_chdir("../../");
}

void MeshMgr_CAccess::meshTexIDs(int meshID) const
{
	// do some boundary testing
	if (meshID < 0 || meshID >= mesh.meshList.size()) {
		console.addLine("     mesh ID %i not found", meshID);
		return;
	}

	console.addLine("     mesh \"%s\" textures", mesh.getMesh(meshID).filename.c_str());

	for (int m = 0; m < mesh.getMesh(meshID).numMaterials; ++m) {
		console.addLine("          texture ID: %i", mesh.getMesh(meshID).materials[m].textureID);
	}
}

void MeshMgr_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if		(_stricmp(_callname,"meshlist")==0) meshList();
	else if	(_stricmp(_callname,"meshload")==0) meshLoad(_varlist[0]->s->c_str(),_varlist[1]->b);
	else if	(_stricmp(_callname,"meshtexids")==0) meshTexIDs(_varlist[0]->i);
}


MeshMgr_CAccess::MeshMgr_CAccess()
{
	console.registerCommand("meshlist","()",this,0);
	console.registerCommand("meshload","(string inputFile, bool flipYZ)",this,2,CVAR_STRING,CVAR_BOOL);
	console.registerCommand("meshtexids","(int meshID)",this,1,CVAR_INT);
}


MeshMgr_CAccess::~MeshMgr_CAccess()
{
	console.unregisterCommand("meshlist");
	console.unregisterCommand("meshload");
	console.unregisterCommand("meshtexids");
}