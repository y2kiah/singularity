/* ----==== OBJECT.H ====---- */

#ifndef OBJECT_H
#define OBJECT_H

#include "MATHCODE\vector3.h"
#include "UTILITYCODE\color.h"

/*-------------
---- ENUMS ----
-------------*/

// Object animation sequence
typedef enum {
	ANIM_CREEPING = 0,
	ANIM_WALKING,
	ANIM_JOGGING,
	ANIM_SPRINTING,
	ANIM_STAND_TO_CROUCH,
	ANIM_CROUCH_TO_STAND,
	ANIM_CROUCH_TO_CRAWL,
	ANIM_CRAWL_TO_CROUCH,
	ANIM_TOTAL
} tAnimSequence;

// Object sound events
typedef enum {
	SND_OBJ_AMBIENT = 0,
	SND_OBJ_FOOTSTEP,
	SND_OBJ_JUMPING,
	SND_OBJ_LANDING,
	SND_OBJ_ATTACKING,
	SND_OBJ_DEFENDING,
	SND_OBJ_DAMAGED,
	SND_OBJ_DAMAGED_BADLY,
	SND_OBJ_DIEING,
	SND_OBJ_BURNING,
	SND_OBJ_WATER_FLOWING,
	SND_ITEM_PICKUP,
	SND_ITEM_USE,
	SND_ITEM_DROP,
	SND_WEAPON_AMBIENT_LOOP,
	SND_WEAPON_ATTACK,
	SND_WEAPON_DEFEND,
	SND_WEAPON_BRANDISH,
	SND_WEAPON_STORE,
	SND_WEAPON_DROP,
	SND_WEAPON_RELOAD,
	SND_MISSILE_AMBIENT_LOOP,
	SND_MISSILE_HIT,
	SND_MISSILE_DUD,
	SND_TOTAL
} SndEvent;

// This is the general type of object
typedef enum {
	OBJ_TYPE_STATIC = 0,	// The object will not move around in the world, but may still be animated
	OBJ_TYPE_DYNAMIC,		// The object moves around and may be animated, may not be controlled by other objects
	OBJ_TYPE_DYNAMIC_CTRL,	// This can be controlled by a switch or directly by the player (a door)
	OBJ_TYPE_ITEM,			// The object is an inventory item and may not be animated or move on its own
	OBJ_TYPE_SWITCH			// This object can be manipulated to control a DYNAMIC_CTRL object, may be animated
} tObjType;

// These volumes will be tested against the view sphere and view frustum
typedef enum {
    VIS_AABB = 0,		// The terrain quadtree will provide vis testing
    VIS_SPHERE,			// VIS_AABB + A bounding sphere which fits the whole object
    VIS_SUB_OBJECT		// VIS_AABB + VIS_SPHERE + Each sub object of the mesh has a bounding sphere
} tVisibility;

// These volumes will be tested against each other
typedef enum {
    COL_NONE = 0,		// This object will accept no collisions with it
    COL_SPHERE,			// The bounding sphere used for collisions
    COL_OBB,			// Oriented bounding box for collisions
    COL_POLY,			// Polygon perfect collisions
    COL_SUB_OBJECT		// Polygon perfect with sub-object spheres first
} tCollision;


/*------------------
---- STRUCTURES ----
------------------*/

class	Mesh3D_Type;
class	Camera;


class Object_Type {
	friend class ObjectMgr_CAccess;
	
	private:

		std::string		objName;		// object name
//		colVolume		*cVolume;		// Collision volume
//		colVolume		*vVolume;		// Visibility volume
//		tVisibility		vis_test_type;
//		tCollision		col_test_type;
		int				meshID;			// Mesh3D_Type index

		bool			doHWLight;		// hardware lighting?

	public:

		const std::string &	getObjName() const { return objName; }		
		const int			getMeshID() const { return meshID; }

		explicit Object_Type(const char *_name, int _meshID);
		~Object_Type() {}
};


class Object_Instance {
	friend class ObjectMgr_CAccess;

	private:

		Vector3			worldPos;
		int				xAngle, yAngle, zAngle;
		
		int				objTypeID;

	public:
		
		void		setWorldPos(const Vector3 &_p, int _xa, int _ya, int _za) { worldPos = _p; xAngle = _xa; yAngle = _ya; zAngle = _za; }		
		void		render(const Camera &cam) const;

		explicit Object_Instance(int _objType);
		~Object_Instance() {}
};


#endif