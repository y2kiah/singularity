//	----==== MESHMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			3/06
//	Description:	
//	--------------------------------------------------------------------------------


#ifndef MESHMANAGER_H
#define MESHMANAGER_H

#include <vector>
#include "mesh.h"
#include "UTILITYCODE\singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define mesh		MeshManager::instance()	// used to access the MeshManager instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class MeshMgr_CAccess;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class MeshManager : public Singleton<MeshManager> {
	friend class MeshMgr_CAccess;

	private:
		///// Variables

		std::vector<Mesh3D_Type*>	meshList;

		CHandler	*cHandler;

	public:		

		///// Functions

		bool	isEmpty(void) const { return meshList.empty(); }
		int		meshListSize() const { return meshList.size(); }
		int		push(Mesh3D_Type *model);	// Pushes a new mesh to the back of the list, returns ID
		void	pop(void);					// Deletes back mesh from memory
		void	clear(void);				// Deletes all mesh types
		
		const Mesh3D_Type	&getMesh(int m) const;

		///// Constructors / Destructor

		explicit MeshManager();
		~MeshManager();
};


#endif