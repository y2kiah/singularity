//	----==== OBJECTMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			3/06
//	Description:	
//	--------------------------------------------------------------------------------


#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include <vector>
#include "object.h"
#include "UTILITYCODE\singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define objectMgr		ObjectManager::instance()	// used to access the ObjectManager instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class ObjectMgr_CAccess;
class Camera;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class ObjectManager : public Singleton<ObjectManager> {
	friend class ObjectMgr_CAccess;

	private:
		///// Variables

		std::vector<Object_Type*>		objTypeList;
		std::vector<Object_Instance*>	objInstanceList;

		CHandler	*cHandler;

	public:

		void	renderObjects(const Camera &cam) const;

		///// Object Type Functions

		bool	typeListIsEmpty(void) const { return objTypeList.empty(); }
		int		pushType(Object_Type *_objType);			// Pushes a new object type to the back, returns ID
		void	clearTypes(void);							// Deletes all object types
		
		const Object_Type		&getObjectType(int o) const;

		///// Object Instance Functions

		bool	instanceListIsEmpty(void) const { return objInstanceList.empty(); }
		int		pushInstance(Object_Instance *_objInst);	// Pushes a new object to the back, returns ID
		void	clearInstances(void);						// Deletes all objects

		const Object_Instance	&getObjectInstance(int o) const;

		///// Constructors / Destructor

		explicit ObjectManager();
		~ObjectManager();
};


#endif