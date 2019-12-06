/* ----==== MESH.CPP ====---- */

#include <GL\glew.h>
#include "mesh.h"
#include "console.h"
#include "engine.h"
#include "3ds.h"
#include "texture.h"
#include "world.h"
#include "frustum.h"
#include "collision.h"
#include "texturemanager.h"
#include "cameramanager.h"
#include "UTILITYCODE\lookupmanager.h"
#include "UTILITYCODE\msgassert.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Mesh3D_Object //////////


Mesh3D_Object::Mesh3D_Object()
{
	vertices = 0;
	vertexNormals = 0;
	uvCoords = 0;
	vertIndexBuffer = 0;
	uvIndexBuffer = 0;
	objSphere = 0;
}


Mesh3D_Object::~Mesh3D_Object()
{
	delete [] vertices;
	delete [] vertexNormals;
	delete [] uvCoords;
	delete [] vertIndexBuffer;
	delete [] uvIndexBuffer;
	delete objSphere;
}


////////// class Mesh3D_Type //////////


void Mesh3D_Type::calcSpheres(void)
{
	float highestmag = 0;

	for (int i = 0; i < numObjects; ++i) {
		for (int v = 0; v < objects[i].numvertices; ++v) {
			float magsquared = objects[i].vertices[v].magSquared();
			if (magsquared > highestmag) highestmag = magsquared;
		}

		if (numObjects > 1) {
			float highestobjmag = 0;
			Vector3	upper(-INFINITY, -INFINITY, -INFINITY);
			Vector3	lower(INFINITY, INFINITY, INFINITY);

			objects[i].objSphere = new BSphere;

			// Find center point of sub object
			for (int v2 = 0; v2 < objects[i].numvertices; ++v2) {				
				if (objects[i].vertices[v2].x < lower.x) lower.x = objects[i].vertices[v2].x;
				if (objects[i].vertices[v2].y < lower.y) lower.y = objects[i].vertices[v2].y;
				if (objects[i].vertices[v2].z < lower.z) lower.z = objects[i].vertices[v2].z;
				
				if (objects[i].vertices[v2].x > upper.x) upper.x = objects[i].vertices[v2].x;
				if (objects[i].vertices[v2].y > upper.y) upper.y = objects[i].vertices[v2].y;
				if (objects[i].vertices[v2].z > upper.z) upper.z = objects[i].vertices[v2].z;
			}

			objects[i].objSphere->p.add(upper, lower);
			objects[i].objSphere->p *= 0.5f;

			// Find radius of sphere
			for (int v3 = 0; v3 < objects[i].numvertices; ++v3) {
				float objdistsquared = objects[i].objSphere->p.distSquared(objects[i].vertices[v3]);
				if (objdistsquared > highestobjmag) highestobjmag = objdistsquared;
			}

			objects[i].objSphere->radius = sqrtf(highestobjmag);
		}
	}

	bSphere.p.assign(0,0,0);
	bSphere.radius = sqrtf(highestmag);
}


colTestType Mesh3D_Type::checkVisible(const Vector3 &wp, int xa, int ya, int za, const Camera &cam) const
{
	// get world space bounding sphere
	BSphere worldSphere(bSphere.p, bSphere.radius);
	//worldSphere.p.rot3D(xa,ya,za); // put in when using individual object spheres
	worldSphere.p += wp;
	
	// check bounding sphere against frustum sphere
	if (worldSphere.collideWithSphere(cam.getFrustum().getWorldSphere()) == COLLISION_FALSE) return COLLISION_FALSE;

	// check bounding sphere against frustum planes
	const colTestType returnval = cam.getFrustum().sphereInsidePlanes(worldSphere);

	// if dev view render bounding sphere axes
	if (returnval && engine.cOptions.cameraView) {
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_LINES);
			glColor3f(1.0f, 1.0f, 1.0f);

			glVertex3f(worldSphere.p.x-worldSphere.radius, worldSphere.p.y, worldSphere.p.z);
			glVertex3f(worldSphere.p.x+worldSphere.radius, worldSphere.p.y, worldSphere.p.z);

			glVertex3f(worldSphere.p.x, worldSphere.p.y-worldSphere.radius, worldSphere.p.z);
			glVertex3f(worldSphere.p.x, worldSphere.p.y+worldSphere.radius, worldSphere.p.z);

			glVertex3f(worldSphere.p.x, worldSphere.p.y, worldSphere.p.z-worldSphere.radius);
			glVertex3f(worldSphere.p.x, worldSphere.p.y, worldSphere.p.z+worldSphere.radius);
		glEnd();
	}

	return returnval;
}


void Mesh3D_Type::render(const Vector3 &wp, int xa, int ya, int za, const Camera &cam) const
{
	colTestType visibility = checkVisible(wp, xa, ya, za, cam);
	if (visibility == COLLISION_FALSE) return;

	glEnable(GL_LIGHTING); // use object lighting property when implemented

	glPushMatrix();	// push current modelview matrix

	glTranslatef(wp.x, wp.y, wp.z);	// translate object space -> world space

	glRotatef(math.indexToDeg(za),0,0,1);	// rotate to world space coords
	glRotatef(math.indexToDeg(xa),1,0,0);
	glRotatef(-math.indexToDeg(ya),0,1,0);

	// render mesh
	for (int i = 0; i < numObjects; i++) {
		Texture const *usetex = 0;
		bool isAlphaTexture = false, hasMaterial = false, hasTexture = false;

		if (objects[i].material_id != -1) {
			hasMaterial = true;
			if (materials[objects[i].material_id].textureID != -1) {
				hasTexture = true;
				glColor3f(1.0f,1.0f,1.0f);
			} else {
				glColor3f(materials[objects[i].material_id].c.r, materials[objects[i].material_id].c.g, materials[objects[i].material_id].c.b);
			}
		} else {
			glColor3ub(255,255,255);
		}

		if (hasTexture) {
			glEnable(GL_TEXTURE_2D);
		
			usetex = &texture.getTexture(materials[objects[i].material_id].textureID);
			
			isAlphaTexture = usetex->getIsAlpha();
			
			if (!isAlphaTexture) glBindTexture(GL_TEXTURE_2D, usetex->getGLTexID());

		} else {
			glDisable(GL_TEXTURE_2D);
		}

		if (!isAlphaTexture) {

			glBegin(GL_TRIANGLES);
			
				for (int j = 0; j < objects[i].numfaces*3; j++) {

					const int vertindex = objects[i].vertIndexBuffer[j];
					glNormal3fv(objects[i].vertexNormals[vertindex].v);
				
					if (hasTexture) {
						const int uvindex = objects[i].uvIndexBuffer[j];
						glTexCoord2f(objects[i].uvCoords[vertindex].u, objects[i].uvCoords[vertindex].v);
					}

					glVertex3f(objects[i].vertices[vertindex].x,
							   objects[i].vertices[vertindex].y,
							   objects[i].vertices[vertindex].z);
				}

			glEnd();

		} else {
			// if it is an alpha texture
			// do not use supplied camera (cam) for this part, transparent tris should always be sorted by the active cam
			for (int j = 0; j < objects[i].numfaces*3; j+=3) {
				
				Vector3 triVert[3], triMidpoint(0,0,0);
				TexCoord triCoord[3];

				for (int v = 0; v < 3; v++) {
					int vertIndex = objects[i].vertIndexBuffer[j+v];
					int uvIndex = objects[i].uvIndexBuffer[j+v];

					// assign object coordinates
					triVert[v].assign(objects[i].vertices[vertIndex].x,
												objects[i].vertices[vertIndex].y,
												objects[i].vertices[vertIndex].z);
					// move to world space
					triVert[v].rot3D(xa, ya, za);
					triVert[v] += wp;

					// sum for midpoint
					triMidpoint += triVert[v];

					triCoord[v].u = objects[i].uvCoords[vertIndex].u;
					triCoord[v].v = objects[i].uvCoords[vertIndex].v;
				}

				// Backface culling here will prevent a lot of faces from making it
				Plane3 newplane(triVert[0], triVert[1], triVert[2]);

				if (newplane.findDist(cam.getP()) >= 0.0f) {
					TransparentTri newTri;

					triMidpoint *= 0.333333333f; // find the midpoint by getting average of all vertices
					float distToCam = camera.getActiveCam().getP().distSquared(triMidpoint);

					newTri.distToCam = distToCam;
					newTri.texID = materials[objects[i].material_id].textureID;
			
					for (int i = 0; i < 3; i++) {
						newTri.tCoord[i].u = triCoord[i].u;
						newTri.tCoord[i].v = triCoord[i].v;
						newTri.vert[i].assign(triVert[i].x, triVert[i].y, triVert[i].z);
					}
						
					camera.getActiveCam().getTransTris().push(newTri);
				}
			}
		}
	}

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glPopMatrix(); // restore modelview matrix
}


bool Mesh3D_Type::load3DS(const std::string &_filename, bool swapYZ)
{
	Model_3DS *model3DS = new Model_3DS;
	
	if (!model3DS->load(_filename, swapYZ)) {
		delete model3DS;
		return false;
	}

	filename = _filename;

	///// Begin conversion to internal mesh format

	numMaterials = model3DS->numOfMaterials;
	numObjects = model3DS->numOfObjects;

	// Copy material data
	materials = new Mesh3D_Material[numMaterials];
	for (int m = 0; m < numMaterials; m++) {
		materials[m].c.r = model3DS->pMaterial[m].color[0];
		materials[m].c.g = model3DS->pMaterial[m].color[1];
		materials[m].c.b = model3DS->pMaterial[m].color[2];
		materials[m].c.a = 1;
		materials[m].textureID = model3DS->pMaterial[m].textureID;
	}

	// Copy object data
	objects = new Mesh3D_Object[numObjects];

	for (int o = 0; o < numObjects; o++) {
		objects[o].material_id = model3DS->pObject[o]->materialID;
		objects[o].numvertices = model3DS->pObject[o]->numOfVerts;
		objects[o].numuvcoords = model3DS->pObject[o]->numTexVertex;
		objects[o].numfaces = model3DS->pObject[o]->numOfFaces;

		objects[o].vertices = new Vector3[objects[o].numvertices];
		objects[o].vertexNormals = new Vector3[objects[o].numvertices];
		objects[o].uvCoords = new TexCoord[objects[o].numuvcoords];
		objects[o].vertIndexBuffer = new unsigned int[objects[o].numfaces * 3];
		objects[o].uvIndexBuffer = new unsigned int[objects[o].numfaces * 3];


		// Copy vertex and normal data
		for (int v = 0; v < objects[o].numvertices; v++) {
			//objects[o].vertices[v] = model3DS->pObject[o].pVerts[v];
			
			objects[o].vertices[v].assign(model3DS->pObject[o]->pVerts[v].x,
											model3DS->pObject[o]->pVerts[v].y,
											model3DS->pObject[o]->pVerts[v].z);
			
			objects[o].vertexNormals[v] = model3DS->pObject[o]->pNormals[v];
		}

		// Copy texture coordinate data
		for (int t = 0; t < objects[o].numuvcoords; t++) {
			objects[o].uvCoords[t] = model3DS->pObject[o]->pTexVerts[t];
		}

		// Copy face data
		for (int f = 0; f < objects[o].numfaces; f++) {
			objects[o].vertIndexBuffer[f*3]   = model3DS->pObject[o]->pFaces[f].vertIndex[0];
			objects[o].vertIndexBuffer[f*3+1] = model3DS->pObject[o]->pFaces[f].vertIndex[1];
			objects[o].vertIndexBuffer[f*3+2] = model3DS->pObject[o]->pFaces[f].vertIndex[2];

			objects[o].uvIndexBuffer[f*3]   = model3DS->pObject[o]->pFaces[f].coordIndex[0];
			objects[o].uvIndexBuffer[f*3+1] = model3DS->pObject[o]->pFaces[f].coordIndex[1];
			objects[o].uvIndexBuffer[f*3+2] = model3DS->pObject[o]->pFaces[f].coordIndex[2];
		}
	}

	/////
	
	delete model3DS;

	calcSpheres();

	return true;
}


Mesh3D_Type::Mesh3D_Type()
{
	numMaterials = numObjects = 0;
	materials = 0;
	objects = 0;
	filename = "";
}


Mesh3D_Type::~Mesh3D_Type()
{
	delete [] materials;
	delete [] objects;
}
