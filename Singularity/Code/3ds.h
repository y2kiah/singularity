/* ----==== 3DS.H ====---- */

#ifndef _3DS_H
#define _3DS_H

#include <vector>
#include <string>
#include "MATHCODE\vector3.h"

// Primary Chunk, at the beginning of each file
#define PRIMARY			0x4D4D

// Main Chunks
#define OBJECTINFO		0x3D3D				// This gives the version of the mesh and is found right before the material and object information
#define VERSION			0x0002				// This gives the version of the .3ds file
#define EDITKEYFRAME	0xB000				// This is the header for all of the key frame info

// sub defines of OBJECTINFO
#define MATERIAL		0xAFFF				// This stored the texture info
#define OBJECT			0x4000				// This stores the faces, vertices, etc...

// sub defines of MATERIAL
#define MATNAME			0xA000				// This holds the material name
#define MATDIFFUSE		0xA020				// This holds the color of the object/material
#define MATMAP			0xA200				// This is a header for a new material
#define MATMAPFILE		0xA300				// This holds the file name of the texture

#define OBJECT_MESH		0x4100				// This lets us know that we are reading a new object

// sub defines of OBJECT_MESH
#define OBJECT_VERTICES		0x4110			// The objects vertices
#define OBJECT_FACES		0x4120			// The objects faces
#define OBJECT_MATERIAL		0x4130			// This is found if the object has a material, either texture map or color
#define OBJECT_UV			0x4140			// The UV texture coordinates

/*------------------
---- STRUCTURES ----
------------------*/

struct	TexCoord;
class	Texture;
class	Load_3DS;


struct Face_3DS {
	int vertIndex[3];			// indicies for the verts that make up this triangle
	int coordIndex[3];			// indicies for the tex coords to texture this face
};


struct Indices_3DS {							
	unsigned short	a, b, c, bVisible;		// This will hold point1, 2, and 3 index's into the vertex array plus a visible flag
};


struct Chunk_3DS {
	unsigned short	ID;				// The chunk's ID		
	unsigned int	length;			// The length of the chunk
	unsigned int	bytesRead;		// The amount of bytes read within that chunk
};


struct MaterialInfo_3DS {
	char			strName[255];	// The texture name
	char			strFile[255];	// The texture file name (If this is set it's a texture map)
	unsigned char	color[3];		// The color of the object (R, G, B)
	int				textureID;		// the texture ID
	float			uTile;			// u tiling of texture  (Currently not used)
	float			vTile;			// v tiling of texture	(Currently not used)
	float			uOffset;	    // u offset of texture	(Currently not used)
	float			vOffset;		// v offset of texture	(Currently not used)
};


class Object_3DS {
	public:
		int				numOfVerts;		// The number of verts in the model
		int				numOfFaces;		// The number of faces in the model
		int				numTexVertex;	// The number of texture coordinates
		int				materialID;		// The material to use, which is the proper index into the TextureList
		bool			bHasTexture;	// This is TRUE if there is a texture map for this object
		char			strName[255];	// The name of the object
		Vector3			*pVerts;		// The object's vertices
		Vector3			*pNormals;		// The object's normals
		TexCoord		*pTexVerts;		// The texture's UV coordinates
		Face_3DS		*pFaces;		// The faces information of the object

		explicit Object_3DS();
		~Object_3DS();
};


class Model_3DS {
	friend class Load_3DS;
	friend class Mesh3D_Type;

	private:
		std::vector<MaterialInfo_3DS>	pMaterial;	// The list of material information (Textures and colors)
		std::vector<Object_3DS*>		pObject;	// The object list for our model
		
		int		numOfObjects;						// The number of objects in the model
		int		numOfMaterials;						// The number of materials for the model

	public:
		std::string	filename;

		bool	load(const std::string &_filename, bool swapYZ);

		explicit Model_3DS();
		~Model_3DS();
};


class Load_3DS {
	private:
		FILE		*m_FilePointer;

		Chunk_3DS	*m_CurrentChunk;
		Chunk_3DS	*m_TempChunk;

		bool	swapYZ;
		
		int		GetString(char *);
		void	ProcessNextChunk(Model_3DS *pModel, Chunk_3DS *);
		void	ProcessNextObjectChunk(Model_3DS *pModel, Object_3DS *pObject, Chunk_3DS *);
		void	ProcessNextMaterialChunk(Model_3DS *pModel, Chunk_3DS *);
		void	ReadChunk(Chunk_3DS *);
		void	ReadColorChunk(MaterialInfo_3DS *pMaterial, Chunk_3DS *pChunk);
		void	ReadVertices(Object_3DS *pObject, Chunk_3DS *);
		void	ReadVertexIndices(Object_3DS *pObject, Chunk_3DS *);
		void	ReadUVCoordinates(Object_3DS *pObject, Chunk_3DS *);
		void	ReadObjectMaterial(Model_3DS *pModel, Object_3DS *pObject, Chunk_3DS *pPreviousChunk);
		void	ComputeNormals(Model_3DS *pModel);
		void	CleanUp();
	
	public:
		bool	import3DS(Model_3DS *pModel, const std::string &_filename, bool _swapYZ);

		explicit Load_3DS();
		~Load_3DS();
};

#endif
