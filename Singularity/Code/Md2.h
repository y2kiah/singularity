/* ----==== MD2.H ====---- */

#ifndef MD2_H
#define MD2_H

/*---------------
---- DEFINES ----
---------------*/

// These are the needed defines for the max values when loading .MD2 files
#define MD2_MAX_TRIANGLES		4096
#define MD2_MAX_VERTICES		2048
#define MD2_MAX_TEXCOORDS		2048
#define MD2_MAX_FRAMES			512
#define MD2_MAX_SKINS			32
#define MD2_MAX_FRAMESIZE		(MD2_MAX_VERTICES * 4 + 128)
//#define kAnimationSpeed			5.0f

/*------------------
---- STRUCTURES ----
------------------*/

// This holds the header information that is read in at the beginning of the file
struct MD2Header { 
	int magic;					// This is used to identify the file
	int version;				// The version number of the file (Must be 8)
	int skinWidth;				// The skin width in pixels
	int skinHeight;				// The skin height in pixels
	int frameSize;				// The size in bytes the frames are
	int numSkins;				// The number of skins associated with the model
	int numVertices;			// The number of vertices (constant for each frame)
	int numTexCoords;			// The number of texture coordinates
	int numTriangles;			// The number of faces (polygons)
	int numGlCommands;			// The number of gl commands
	int numFrames;				// The number of animation frames
	int offsetSkins;			// The offset in the file for the skin data
	int offsetTexCoords;		// The offset in the file for the texture data
	int offsetTriangles;		// The offset in the file for the face data
	int offsetFrames;			// The offset in the file for the frames data
	int offsetGlCommands;		// The offset in the file for the gl commands data
	int offsetEnd;				// The end of the file offset
};

// This is used to store the vertices that are read in for the current frame
struct MD2AliasTriangle {
	unsigned char vertex[3];
	unsigned char lightNormalIndex;
};

// This stores the normals and vertices for the frames
struct MD2Triangle {
	float vertex[3];
	float normal[3];
};

// This stores the indices into the vertex and texture coordinate arrays
struct MD2Face {
	short vertexIndices[3];
	short textureIndices[3];
};

// This stores UV coordinates
struct MD2TexCoord {
	short u, v;
};

// This stores the animation scale, translation and name information for a frame, plus verts
struct MD2AliasFrame {
	float				scale[3];
	float				translate[3];
	char				name[16];
	MD2AliasTriangle	aliasVertices[1];
};

// This stores the frames vertices after they have been transformed
struct MD2Frame {
	char			strName[16];
	MD2Triangle		*pVertices;
};

// This stores a skin name
typedef char MD2Skin[64];

// This class handles all of the loading code
class MD2File {
	private:
		MD2Header		Header;			// The header data
		MD2Skin			*Skins;			// The skin data
		MD2TexCoord		*TexCoords;		// The texture coordinates
		MD2Face			*Triangles;		// Face index information
		MD2Frame		*Frames;		// The frames of animation (vertices)

		// This reads in the data from the MD2 file and stores it in the member variables
//		void ReadMD2Data();
//		void ParseAnimations(t3DModel *pModel);
//		void ConvertDataStructures(t3DModel *pModel);
//		bool ImportMD2(/*t3DModel *pModel,*/ char *strFileName, char *strTexture)
//		void CleanUp();

	public:
		MD2File();						// This inits the data members

};

#endif