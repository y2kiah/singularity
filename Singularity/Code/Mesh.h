/* ----==== MESH.H ====---- */

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include "collision.h"
#include "texcoord.h"
#include "MATHCODE\vector3.h"
#include "UTILITYCODE\color.h"


/*---------------
---- DEFINES ----
---------------*/

#define	FRAMES_PER_SEQ		5

// Animation sequences, to be matched up with key frames.
// Each mesh has an array size SEQ_TOTAL with a number in each
// representing the keyframe to use for each particular animation
enum {
	SEQ_STAND = 0,
	SEQ_WALK,
	SEQ_RUN,
	SEQ_JUMP,
	SEQ_CROUCH,
	SEQ_ATTACK1,
	SEQ_ATTACK2,
	SEQ_ATTACK3,
	SEQ_DEFEND1,
	SEQ_DEFEND2,
	SEQ_DEFEND3,
	SEQ_TALK,
	SEQ_MISC1,
	SEQ_MISC2,
	SEQ_MISC3,
	SEQ_DIE1,
	SEQ_DIE2,
	SEQ_DIE3,
	SEQ_TOTAL
};

/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class MeshMgr_CAccess;
class Mesh3D_Type;
class Camera;


class Mesh3D_Material {
	public:
		Color4f			c;
		int				textureID;			// if -1 then no texture and use color
};


class Mesh3D_Object {
	friend class Mesh3D_Type;

	private:
		Vector3			*vertices;
		Vector3			*vertexNormals;
		TexCoord		*uvCoords;			// Texture mapping coordinates

		int				material_id;		// if -1 use color4f(1,1,1,1)
		unsigned int	numvertices;
		unsigned int	numuvcoords;
		unsigned int	numfaces;

		unsigned int	*vertIndexBuffer;	// Index values into the vertex buffer, specifies triangles in object
		unsigned int	*uvIndexBuffer;		// Texture index values

		BSphere			*objSphere;			// Object bounding sphere, created only if there are 2 or more objects

	public:
		explicit Mesh3D_Object();
		~Mesh3D_Object();
};


class Mesh3D_Type {
	friend class MeshMgr_CAccess;

	private:
		Mesh3D_Material		*materials;			// All materials for this object
		Mesh3D_Object		*objects;			// Stores faces sorted by material id for quickest rendering

		std::string			filename;
		int					numMaterials;
		int					numObjects;

//		colAABB				bBox;				// AABB (angle 0,0,0) to fit all frames
		BSphere				bSphere;			// Bounding sphere fits all frames

		void				calcSpheres(void);	// Calculate main and object bounding spheres

	public:		
		////// TEMP
		void				render(const Vector3 &wp, int xa, int ya, int za, const Camera &cam) const;
		colTestType			checkVisible(const Vector3 &wp, int xa, int ya, int za, const Camera &cam) const;
		///////////

//		bool				loadM3D(const char *_filename) {}	// Load native M3D format
//		bool				loadX(const char *_filename) {}
//		bool				loadMD2(const char *_filename) {}
		bool				load3DS(const std::string &_filename, bool swapYZ);

		const std::string &		getFilename(void) const		{ return filename; }
		const std::string &		toString(void) const		{ return filename; }

		explicit Mesh3D_Type();
		~Mesh3D_Type();
};

#endif