/* ----==== 3DS.CPP ====---- */

#include <GL\glew.h>
#include <cstdio>
#include <cmath>
#include "console.h"
#include "3ds.h"
#include "texcoord.h"
//#include "quadtree.h"
#include "texture.h"
//#include "engine.h"
//#include "camera.h"
//#include "world.h"
#include "texturemanager.h"
#include "MATHCODE\plane3.h"


/*-----------------
---- VARIABLES ----
-----------------*/

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Object_3DS //////////

Object_3DS::Object_3DS()
{
	numOfVerts = numOfFaces = numTexVertex = 0;
	materialID = -1;
	bHasTexture = false;
	strName[0] = 0;
	pVerts = 0;
	pNormals = 0;
	pTexVerts = 0;
	pFaces = 0;
}


Object_3DS::~Object_3DS()
{
	delete [] pVerts;
	delete [] pNormals;
	delete [] pTexVerts;
	delete [] pFaces;	
}


////////// class Model_3DS //////////

bool Model_3DS::load(const std::string &_filename, bool swapYZ)
{
	bool returnval = true;

	Load_3DS *loader = new Load_3DS;
	returnval = loader->import3DS(this, _filename, swapYZ);

	delete loader;

	return returnval;
}


Model_3DS::Model_3DS()
{
	numOfObjects = numOfMaterials = 0;
	filename = "";
}


Model_3DS::~Model_3DS()
{
	for (int i = 0; i < pObject.size(); i++) { delete pObject[i]; }
	pObject.clear();
	pMaterial.clear();
}


////////// class Load_3DS //////////


int Load_3DS::GetString(char *pBuffer)
{
	int index = 0;

	// Read 1 byte of data which is the first letter of the string
	fread(pBuffer, 1, 1, m_FilePointer);

	// Loop until we get NULL
	while (*(pBuffer + index++) != 0) {

		// Read in a character at a time until we hit NULL.
		fread(pBuffer + index, 1, 1, m_FilePointer);
	}

	// Return the string length, which is how many bytes we read in (including the NULL)
	return strlen(pBuffer) + 1;
}


void Load_3DS::ProcessNextChunk(Model_3DS *pModel, Chunk_3DS *pPreviousChunk)
{	
	unsigned int		version = 0;				// This will hold the file version
	int					buffer[50000] = {0};		// This is used to read past unwanted data

	m_CurrentChunk = new Chunk_3DS;				// Allocate a new chunk	

	// Below we check our chunk ID each time we read a new chunk.  Then, if
	// we want to extract the information from that chunk, we do so.
	// If we don't want a chunk, we just read past it.

	// Continue to read the sub chunks until we have reached the length.
	// After we read ANYTHING we add the bytes read to the chunk and then check
	// check against the length.
	while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
		MaterialInfo_3DS newMaterial = {0};
		Object_3DS *newObject;
		
		// Read next Chunk
		ReadChunk(m_CurrentChunk);

		// Check the chunk ID
		switch (m_CurrentChunk->ID) {
			case VERSION:							// This holds the version of the file
				// This chunk has an unsigned short that holds the file version.
				// Since there might be new additions to the 3DS file format in 4.0,
				// we give a warning to that problem.

				// Read the file version and add the bytes read to our bytesRead variable
				m_CurrentChunk->bytesRead += fread(&version, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);

				// If the file version is over 3, give a warning that there could be a problem
				if (version > 0x03)
					console.addLineErr("     this 3DS file is over version 3 so it may load incorrectly");
				break;

			case OBJECTINFO:						// This holds the version of the mesh		
				// This chunk holds the version of the mesh.  It is also the head of the MATERIAL
				// and OBJECT chunks.  From here on we start reading in the material and object info.

				// Read the next chunk
				ReadChunk(m_TempChunk);

				// Get the version of the mesh
				m_TempChunk->bytesRead += fread(&version, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

				// Increase the bytesRead by the bytes read from the last chunk
				m_CurrentChunk->bytesRead += m_TempChunk->bytesRead;

				// Go to the next chunk, which is the object has a texture, it should be MATERIAL, then OBJECT.
				ProcessNextChunk(pModel, m_CurrentChunk);
				break;

			case MATERIAL:							// This holds the material information
				// This chunk is the header for the material info chunks

				pModel->numOfMaterials++;

				pModel->pMaterial.push_back(newMaterial);

				// Proceed to the material loading function
				ProcessNextMaterialChunk(pModel, m_CurrentChunk);
				break;

			case OBJECT:							// This holds the name of the object being read				
				// This chunk is the header for the object info chunks.  It also
				// holds the name of the object.

				pModel->numOfObjects++;
		
				newObject = new Object_3DS;		// This is used to add to our object list
				pModel->pObject.push_back(newObject);
			
				// Get the name of the object and store it, then add the read bytes to our byte counter.
				m_CurrentChunk->bytesRead += GetString(newObject->strName);
			
				// Now proceed to read in the rest of the object information
				ProcessNextObjectChunk(pModel, newObject, m_CurrentChunk);
				break;

			case EDITKEYFRAME:
				// Because I wanted to make this a SIMPLE tutorial as possible, I did not include
				// the key frame information.  This chunk is the header for all the animation info.
				// In a later tutorial this will be the subject and explained thoroughly.

				//ProcessNextKeyFrameChunk(pModel, m_CurrentChunk);

				// Read past this chunk and add the bytes read to the byte counter
				m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;

			default: 		
				// If we didn't care about a chunk, then we get here.  We still need
				// to read past the unknown or ignored chunk and add the bytes read to the byte counter.
				m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// Free the current chunk and set it back to the previous chunk (since it started that way)
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}


void Load_3DS::ProcessNextObjectChunk(Model_3DS *pModel, Object_3DS *pObject, Chunk_3DS *pPreviousChunk)
{
	int buffer[50000] = {0};					// This is used to read past unwanted data

	// Allocate a new chunk to work with
	m_CurrentChunk = new Chunk_3DS;

	// Continue to read these chunks until we read the end of this sub chunk
	while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
		// Read the next chunk
		ReadChunk(m_CurrentChunk);

		// Check which chunk we just read
		switch (m_CurrentChunk->ID) {
			case OBJECT_MESH:					// This lets us know that we are reading a new object	
				// We found a new object, so let's read in it's info using recursion
				ProcessNextObjectChunk(pModel, pObject, m_CurrentChunk);
				break;

			case OBJECT_VERTICES:				// This is the objects vertices
				ReadVertices(pObject, m_CurrentChunk);
				break;

			case OBJECT_FACES:					// This is the objects face information
				ReadVertexIndices(pObject, m_CurrentChunk);
				break;

			case OBJECT_MATERIAL:				// This holds the material name that the object has		
				// This chunk holds the name of the material that the object has assigned to it.
				// This could either be just a color or a texture map.  This chunk also holds
				// the faces that the texture is assigned to (In the case that there is multiple
				// textures assigned to one object, or it just has a texture on a part of the object.
				// Since most of my game objects just have the texture around the whole object, and 
				// they aren't multitextured, I just want the material name.

				// We now will read the name of the material assigned to this object
				ReadObjectMaterial(pModel, pObject, m_CurrentChunk);			
				break;

			case OBJECT_UV:						// This holds the UV texture coordinates for the object
				// This chunk holds all of the UV coordinates for our object.  Let's read them in.
				ReadUVCoordinates(pObject, m_CurrentChunk);
				break;

			default:
				// Read past the ignored or unknown chunks
				m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// Free the current chunk and set it back to the previous chunk (since it started that way)
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}


void Load_3DS::ProcessNextMaterialChunk(Model_3DS *pModel, Chunk_3DS *pPreviousChunk)
{
	int buffer[50000] = {0};					// This is used to read past unwanted data

	// Allocate a new chunk to work with
	m_CurrentChunk = new Chunk_3DS;

	// Continue to read these chunks until we read the end of this sub chunk
	while (pPreviousChunk->bytesRead < pPreviousChunk->length) {
		// Read the next chunk
		ReadChunk(m_CurrentChunk);

		// Check which chunk we just read in
		switch (m_CurrentChunk->ID) {
			case MATNAME:							// This chunk holds the name of the material
				m_CurrentChunk->bytesRead += fread(pModel->pMaterial[pModel->numOfMaterials - 1].strName, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;

			case MATDIFFUSE:						// This holds the R G B color of our object
				ReadColorChunk(&(pModel->pMaterial[pModel->numOfMaterials - 1]), m_CurrentChunk);
				break;
		
			case MATMAP:							// This is the header for the texture info
				// Proceed to read in the material information
				ProcessNextMaterialChunk(pModel, m_CurrentChunk);
				break;

			case MATMAPFILE:						// This stores the file name of the material
				// Here we read in the material's file name
				m_CurrentChunk->bytesRead += fread(pModel->pMaterial[pModel->numOfMaterials - 1].strFile, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;
		
			default:  
				// Read past the ignored or unknown chunks
				m_CurrentChunk->bytesRead += fread(buffer, 1, m_CurrentChunk->length - m_CurrentChunk->bytesRead, m_FilePointer);
				break;
		}

		// Add the bytes read from the last chunk to the previous chunk passed in.
		pPreviousChunk->bytesRead += m_CurrentChunk->bytesRead;
	}

	// Free the current chunk and set it back to the previous chunk (since it started that way)
	delete m_CurrentChunk;
	m_CurrentChunk = pPreviousChunk;
}


void Load_3DS::ReadChunk(Chunk_3DS *pChunk)
{
	// This reads the chunk ID which is 2 bytes.
	// The chunk ID is like OBJECT or MATERIAL.  It tells what data is
	// able to be read in within the chunks section.  
	pChunk->bytesRead = fread(&pChunk->ID, 1, 2, m_FilePointer);

	// Then, we read the length of the chunk which is 4 bytes.
	// This is how we know how much to read in, or read past.
	pChunk->bytesRead += fread(&pChunk->length, 1, 4, m_FilePointer);
}


void Load_3DS::ReadColorChunk(MaterialInfo_3DS *pMaterial, Chunk_3DS *pChunk)
{
	// Read the color chunk info
	ReadChunk(m_TempChunk);

	// Read in the R G B color (3 bytes - 0 through 255)
	m_TempChunk->bytesRead += fread(pMaterial->color, 1, m_TempChunk->length - m_TempChunk->bytesRead, m_FilePointer);

	// Add the bytes read to our chunk
	pChunk->bytesRead += m_TempChunk->bytesRead;
}


void Load_3DS::ReadVertices(Object_3DS *pObject, Chunk_3DS *pPreviousChunk)
{
	// Read in the number of vertices (int)
	pPreviousChunk->bytesRead += fread(&(pObject->numOfVerts), 1, 2, m_FilePointer);

	pObject->pVerts = new Vector3[pObject->numOfVerts];

	// Read in the array of vertices (an array of 3 floats)
	pPreviousChunk->bytesRead += fread(pObject->pVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);

	// Go through all of the vertices that we just read and swap the Y and Z values
	if (swapYZ) {
		for(int i = 0; i < pObject->numOfVerts; i++) {
			float fTempY = pObject->pVerts[i].y;
			pObject->pVerts[i].y = pObject->pVerts[i].z;
			// set Y to negative Z because 3D Studio max does the opposite.
			pObject->pVerts[i].z = -fTempY;
		}
	}
}


void Load_3DS::ReadVertexIndices(Object_3DS *pObject, Chunk_3DS *pPreviousChunk)
{
	unsigned short index = 0;					// This is used to read in the current face index

	// Read in the number of faces that are in this object (int)
	pPreviousChunk->bytesRead += fread(&pObject->numOfFaces, 1, 2, m_FilePointer);

	// Alloc enough memory for the faces and initialize the structure
	pObject->pFaces = new Face_3DS[pObject->numOfFaces];
	//memset(pObject->pFaces, 0, sizeof(Face_3DS) * pObject->numOfFaces);

	// Go through all of the faces in this object
	for (int i = 0; i < pObject->numOfFaces; i++) {
		// Next, we read in the A then B then C index for the face, but ignore the 4th value.
		// The fourth value is a visibility flag for 3D Studio Max, we don't care about this.
		for (int j = 0; j < 4; j++) {
			// Read the first vertice index for the current face 
			pPreviousChunk->bytesRead += fread(&index, 1, sizeof(index), m_FilePointer);

			if (j < 3) {
				// Store the index in our face structure.
				pObject->pFaces[i].vertIndex[j] = index;
			}
		}
	}
}


void Load_3DS::ReadUVCoordinates(Object_3DS *pObject, Chunk_3DS *pPreviousChunk)
{
	// Read in the number of UV coordinates there are (int)
	pPreviousChunk->bytesRead += fread(&pObject->numTexVertex, 1, 2, m_FilePointer);

	// Allocate memory to hold the UV coordinates
	pObject->pTexVerts = new TexCoord[pObject->numTexVertex];

	// Read in the texture coodinates (an array 2 float)
	pPreviousChunk->bytesRead += fread(pObject->pTexVerts, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}


void Load_3DS::ReadObjectMaterial(Model_3DS *pModel, Object_3DS *pObject, Chunk_3DS *pPreviousChunk)
{
	char strMaterial[255] = {0};			// This is used to hold the objects material name
	int buffer[50000] = {0};				// This is used to read past unwanted data

	pPreviousChunk->bytesRead += GetString(strMaterial);

	// Go through all of the textures
	for(int i = 0; i < pModel->numOfMaterials; i++) {

		// If the material we just read in matches the current texture name
		if(_stricmp(strMaterial, pModel->pMaterial[i].strName) == 0) {
			// Set the material ID to the current index 'i' and stop checking
			pObject->materialID = i;

			// Now that we found the material, check if it's a texture map.
			// If the strFile has a string length of 1 and over it's a texture
			if (strlen(pModel->pMaterial[i].strFile) > 0) {

				// Set the object's flag to say it has a texture map to bind.
				pObject->bHasTexture = true;
			}

			break;

		} else {
			// Set the ID to -1 to show there is no material for this object
			pObject->materialID = -1;

		}
	}

	// Read past the rest of the chunk since we don't care about shared vertices
	// You will notice we subtract the bytes already read in this chunk from the total length.
	pPreviousChunk->bytesRead += fread(buffer, 1, pPreviousChunk->length - pPreviousChunk->bytesRead, m_FilePointer);
}			


void Load_3DS::ComputeNormals(Model_3DS *pModel)
{
	Vector3 vVector1, vVector2, vNormal, vPoly[3];

	if (pModel->numOfObjects <= 0) return;

	// Go through each of the objects to calculate their normals
	for (int index = 0; index < pModel->numOfObjects; ++index) {
		Object_3DS *pObject = pModel->pObject[index];

		Vector3 *pTempNormals	= new Vector3[pObject->numOfFaces];
		pObject->pNormals		= new Vector3[pObject->numOfVerts];

		///// Get face normals /////
		for (int i = 0; i < pObject->numOfFaces; ++i) {		// Go though all of the faces of this object
			vPoly[0] = pObject->pVerts[pObject->pFaces[i].vertIndex[0]];
			vPoly[1] = pObject->pVerts[pObject->pFaces[i].vertIndex[1]];
			vPoly[2] = pObject->pVerts[pObject->pFaces[i].vertIndex[2]];

			vVector1.subtract(vPoly[0], vPoly[2]);	// Get the vector of the polygon (we just need 2 sides for the normal)
			vVector2.subtract(vPoly[2], vPoly[1]);	// Get a second vector of the polygon

			vNormal.unitNormalOf(vVector1, vVector2);

			pTempNormals[i] = vNormal;				// Assign the normal to the list of normals
		}

		///// Get vertex normals /////
		Vector3 vSum(0,0,0);
		float shared = 0.0f;
		
		for (int i2 = 0; i2 < pObject->numOfVerts; ++i2) {		// Go through all of the vertices
			for (int j = 0; j < pObject->numOfFaces; ++j) {		// Go through all of the triangles
				if (pObject->pFaces[j].vertIndex[0] == i2 ||	// Check if the vertex is shared by another face
					pObject->pFaces[j].vertIndex[1] == i2 || 
					pObject->pFaces[j].vertIndex[2] == i2)
				{
					vSum.add(vSum, pTempNormals[j]);		// Add the normalized normal of the shared face
					shared += 1.0f;							// Increase the number of shared triangles
				}
			}      
						
			// Get normal by dividing sum by shared. Negate shared so it has normals pointing out			
			if (-shared != 0.0f) pObject->pNormals[i2].divide(vSum, -shared);

			// Normalize the final vertex normal
			//pObject->pNormals[i2].normalize();

			vSum.assign(0,0,0);
			shared = 0;
		}
	
		delete [] pTempNormals;
	}
}


bool Load_3DS::import3DS(Model_3DS *pModel, const std::string &_filename, bool _swapYZ)
{
	swapYZ = _swapYZ;

	///// Model loading /////
	if (fopen_s(&m_FilePointer, _filename.c_str(), "rb") != 0) {
		console.addLineErr("     file \"%s\" not found", _filename.c_str());
		return false;
	}

	// Read the first chunk of the file to see if it's a 3DS file
	ReadChunk(m_CurrentChunk);

	// Make sure this is a 3DS file
	if (m_CurrentChunk->ID != PRIMARY) {
		console.addLineErr("     unable to load PRIMARY chunk from file \"%s\"", _filename.c_str());
		return false;
	}

	// Record the file name of the 3ds model
	pModel->filename = _filename;

	// Begin loading objects, by calling this recursive function	
	ProcessNextChunk(pModel, m_CurrentChunk);

	ComputeNormals(pModel);
	fclose(m_FilePointer);

	///// Texture loading /////
	
	// Load the textures	
	for (int c = 0; c < pModel->numOfMaterials; c++) {
		int strLength = strlen(pModel->pMaterial[c].strFile);

		if (strLength > 0) {
			pModel->pMaterial[c].strFile[strLength-3] = 't';
			pModel->pMaterial[c].strFile[strLength-2] = 'g';
			pModel->pMaterial[c].strFile[strLength-1] = 'a';

			pModel->pMaterial[c].textureID = texture.loadFromFile(pModel->pMaterial[c].strFile, GL_REPEAT, GL_MODULATE, true, true, true, false);
			if (pModel->pMaterial[c].textureID == -1) {
				return false;
			}

		} else {
			pModel->pMaterial[c].textureID = -1;
		}
	}

	return true;
}


void Load_3DS::CleanUp()
{
	delete m_CurrentChunk;			// Free the current chunk
	delete m_TempChunk;				// Free our temporary chunk
}


Load_3DS::Load_3DS()
{
	m_CurrentChunk = new Chunk_3DS;
	m_TempChunk = new Chunk_3DS;
}


Load_3DS::~Load_3DS()
{
	CleanUp();
}

// A chunk is defined this way:
// 2 bytes - Stores the chunk ID (OBJECT, MATERIAL, PRIMARY, etc...)
// 4 bytes - Stores the length of that chunk.  That way you know when that
//           chunk is done and there is a new chunk.
//
// So, to start reading the 3DS file, you read the first 2 bytes of it, then
// the length (using fread()).  It should be the PRIMARY chunk, otherwise it isn't
// a .3DS file.  
//
//      MAIN3DS  (0x4D4D)
//     |
//     +--EDIT3DS  (0x3D3D)
//     |  |
//     |  +--EDIT_MATERIAL (0xAFFF)
//     |  |  |
//     |  |  +--MAT_NAME01 (0xA000) (See mli Doc) 
//     |  |
//     |  +--EDIT_CONFIG1  (0x0100)
//     |  +--EDIT_CONFIG2  (0x3E3D) 
//     |  +--EDIT_VIEW_P1  (0x7012)
//     |  |  |
//     |  |  +--TOP            (0x0001)
//     |  |  +--BOTTOM         (0x0002)
//     |  |  +--LEFT           (0x0003)
//     |  |  +--RIGHT          (0x0004)
//     |  |  +--FRONT          (0x0005) 
//     |  |  +--BACK           (0x0006)
//     |  |  +--USER           (0x0007)
//     |  |  +--CAMERA         (0xFFFF)
//     |  |  +--LIGHT          (0x0009)
//     |  |  +--DISABLED       (0x0010)  
//     |  |  +--BOGUS          (0x0011)
//     |  |
//     |  +--EDIT_VIEW_P2  (0x7011)
//     |  |  |
//     |  |  +--TOP            (0x0001)
//     |  |  +--BOTTOM         (0x0002)
//     |  |  +--LEFT           (0x0003)
//     |  |  +--RIGHT          (0x0004)
//     |  |  +--FRONT          (0x0005) 
//     |  |  +--BACK           (0x0006)
//     |  |  +--USER           (0x0007)
//     |  |  +--CAMERA         (0xFFFF)
//     |  |  +--LIGHT          (0x0009)
//     |  |  +--DISABLED       (0x0010)  
//     |  |  +--BOGUS          (0x0011)
//     |  |
//     |  +--EDIT_VIEW_P3  (0x7020)
//     |  +--EDIT_VIEW1    (0x7001) 
//     |  +--EDIT_BACKGR   (0x1200) 
//     |  +--EDIT_AMBIENT  (0x2100)
//     |  +--EDIT_OBJECT   (0x4000)
//     |  |  |
//     |  |  +--OBJ_TRIMESH   (0x4100)      
//     |  |  |  |
//     |  |  |  +--TRI_VERTEXL          (0x4110) 
//     |  |  |  +--TRI_VERTEXOPTIONS    (0x4111)
//     |  |  |  +--TRI_MAPPINGCOORS     (0x4140) 
//     |  |  |  +--TRI_MAPPINGSTANDARD  (0x4170)
//     |  |  |  +--TRI_FACEL1           (0x4120)
//     |  |  |  |  |
//     |  |  |  |  +--TRI_SMOOTH            (0x4150)   
//     |  |  |  |  +--TRI_MATERIAL          (0x4130)
//     |  |  |  |
//     |  |  |  +--TRI_LOCAL            (0x4160)
//     |  |  |  +--TRI_VISIBLE          (0x4165)
//     |  |  |
//     |  |  +--OBJ_LIGHT    (0x4600)
//     |  |  |  |
//     |  |  |  +--LIT_OFF              (0x4620)
//     |  |  |  +--LIT_SPOT             (0x4610) 
//     |  |  |  +--LIT_UNKNWN01         (0x465A) 
//     |  |  | 
//     |  |  +--OBJ_CAMERA   (0x4700)
//     |  |  |  |
//     |  |  |  +--CAM_UNKNWN01         (0x4710)
//     |  |  |  +--CAM_UNKNWN02         (0x4720)  
//     |  |  |
//     |  |  +--OBJ_UNKNWN01 (0x4710)
//     |  |  +--OBJ_UNKNWN02 (0x4720)
//     |  |
//     |  +--EDIT_UNKNW01  (0x1100)
//     |  +--EDIT_UNKNW02  (0x1201) 
//     |  +--EDIT_UNKNW03  (0x1300)
//     |  +--EDIT_UNKNW04  (0x1400)
//     |  +--EDIT_UNKNW05  (0x1420)
//     |  +--EDIT_UNKNW06  (0x1450)
//     |  +--EDIT_UNKNW07  (0x1500)
//     |  +--EDIT_UNKNW08  (0x2200)
//     |  +--EDIT_UNKNW09  (0x2201)
//     |  +--EDIT_UNKNW10  (0x2210)
//     |  +--EDIT_UNKNW11  (0x2300)
//     |  +--EDIT_UNKNW12  (0x2302)
//     |  +--EDIT_UNKNW13  (0x2000)
//     |  +--EDIT_UNKNW14  (0xAFFF)
//     |
//     +--KEYF3DS (0xB000)
//        |
//        +--KEYF_UNKNWN01 (0xB00A)
//        +--............. (0x7001) ( viewport, same as editor )
//        +--KEYF_FRAMES   (0xB008)
//        +--KEYF_UNKNWN02 (0xB009)
//        +--KEYF_OBJDES   (0xB002)
//           |
//           +--KEYF_OBJHIERARCH  (0xB010)
//           +--KEYF_OBJDUMMYNAME (0xB011)
//           +--KEYF_OBJUNKNWN01  (0xB013)
//           +--KEYF_OBJUNKNWN02  (0xB014)
//           +--KEYF_OBJUNKNWN03  (0xB015)  
//           +--KEYF_OBJPIVOT     (0xB020)  
//           +--KEYF_OBJUNKNWN04  (0xB021)  
//           +--KEYF_OBJUNKNWN05  (0xB022)  
