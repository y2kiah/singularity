/* ----==== CAMERA.CPP ====---- */

#include <GL\glew.h>
#include "options.h"
#include "console.h"
#include "cameramgr_console.h"
#include "world.h"
#include "skybox.h"
#include "engine.h"
#include "cameramanager.h"
#include "texturemanager.h"
#include "objectmanager.h"
#include "MATHCODE\vector4.h"
#include "UTILITYCODE\glfont.h"
#include "UTILITYCODE\lookupmanager.h"

//// temp
#include "player.h"	// temp for god view

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class TransparentTriList //////////


void TransparentTriList::fillArrays(void)
{
	// if the list is empty and it wasn't last frame, delete the arrays
	if (listsize == 0 && lastlistsize != 0) {
		delete [] distarray;
		delete [] triarray;
		distarray = 0;
		triarray = 0;
		return;
	}

	// if the listsize is different than it was last frame create new ones with different size
	if (listsize != lastlistsize) {
		delete [] distarray; 
		delete [] triarray;

		distarray = new float[listsize];
		triarray = new TransparentTri*[listsize];
	}
			
	for (int c = 0; c < listsize; c++) {
		distarray[c] = trilist[c].distToCam;
		triarray[c] = &trilist[c];
	}
}


/*void TransparentTriList::performQuickSort(const int left,const int right)
{
	if (right < left) return;

	int i = left, j = right;
	
	float x = distarray[(left+right)/2]; // >> 1 ?
	
	do {
		while (distarray[i] < x && i < right) ++i;
		while (distarray[j] > x && j > left) --j;

		if (i <= j) {
			float tempf = distarray[i];
			distarray[i] = distarray[j];
			distarray[j] = tempf;
			
			TransparentTri *tempt = triarray[i];
			triarray[i] = triarray[j];
			triarray[j] = tempt;
			
			++i;
			--j;
		}
	} while (i <= j);
	
	if (left < j) PerformQuickSort(left,j);
	
	if (i < right) PerformQuickSort(i,right);
}*/


void TransparentTriList::performInsertionSort(void)
{
	for (int i = 1; i < listsize; ++i) {
		const float tempf = distarray[i];
		TransparentTri *tempt = triarray[i];

		int saveJ = i - 1;
		for (int j = i - 1; j >= 0 && distarray[j] > tempf; --j) {
			distarray[j+1] = distarray[j];
			triarray[j+1] = triarray[j];
			saveJ = j;
		}

		distarray[saveJ+1] = tempf;
		triarray[saveJ+1] = tempt;
	}
}


void TransparentTriList::push(TransparentTri &_newtri)
{
	if (listsize == MAX_TRANSPARENT_TRIS - 1) return;
	trilist[listsize] = _newtri;
	++listsize;
}


void TransparentTriList::render(void)
{
	// Pre-rendering sorts
	fillArrays();
	performInsertionSort();

	///// write text here ///////////
/*	glColor3ub(255,255,255);
	glDisable(GL_TEXTURE_2D);
	engine.font->positionText(10,100);
	engine.font->print("%i transparent tris on screen",listsize);
	glEnable(GL_TEXTURE_2D);*/
	/////////////////////////////////

	if (listsize == 0) {
		lastlistsize = 0;
		return;
	}

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);	// list is already backface culled
	glEnable(GL_ALPHA_TEST);	// alpha test is faster than blending, so alpha test first, then blend pixels
	glDepthMask(GL_FALSE);		// they must be depth tested but not written to buffer

	glColor3ub(255,255,255);

	int currentTexID = -1;
	for (int t = listsize-1; t >= 0; --t) {
		TransparentTri *triuse = triarray[t];

		if (triuse->texID != currentTexID) {	// don't bind again if it's the same texture
			glBindTexture(GL_TEXTURE_2D, texture.getTexture(triuse->texID).getGLTexID());
			currentTexID = triuse->texID;
		}

		glBegin(GL_TRIANGLES);
			glTexCoord2f(triuse->tCoord[0].u, triuse->tCoord[0].v);
			glVertex3fv(triuse->vert[0].v);

			glTexCoord2f(triuse->tCoord[1].u, triuse->tCoord[1].v);
			glVertex3fv(triuse->vert[1].v);

			glTexCoord2f(triuse->tCoord[2].u, triuse->tCoord[2].v);
			glVertex3fv(triuse->vert[2].v);
		glEnd();
	}

	glDepthMask(GL_TRUE);
	glDisable(GL_ALPHA_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	// clear the tri list
	lastlistsize = listsize;
	listsize = 0;
}


TransparentTriList::~TransparentTriList()
{
	delete [] distarray;
	delete [] triarray;
}


////////// class Camera //////////


void Camera::setPos(const Vector3 &_p)
{
	p = _p;
}


void Camera::setPosIncrement(const Vector3 &_p)
{
	p += _p;
}


void Camera::setOrientation(int _xa, int _ya, int _za)
{
	xAngle = _xa;
	yAngle = _ya;
	zAngle = _za;

	// do some boundary checking
	if (xAngle < 0) xAngle += math.ANGLE360;
	else if (xAngle >= math.ANGLE360) xAngle -= math.ANGLE360;

	if (yAngle < 0) yAngle += math.ANGLE360;
	else if (yAngle >= math.ANGLE360) yAngle -= math.ANGLE360;

	if (zAngle < 0) zAngle += math.ANGLE360;
	else if (zAngle >= math.ANGLE360) zAngle -= math.ANGLE360;
}


void Camera::setOrientationIncrement(int _xa, int _ya, int _za)
{
	xAngle += _xa;
	yAngle += _ya;
	zAngle += _za;

	// do some boundary checking
	if (xAngle < 0) xAngle += math.ANGLE360;
	else if (xAngle >= math.ANGLE360) xAngle -= math.ANGLE360;

	if (yAngle < 0) yAngle += math.ANGLE360;
	else if (yAngle >= math.ANGLE360) yAngle -= math.ANGLE360;

	if (zAngle < 0) zAngle += math.ANGLE360;
	else if (zAngle >= math.ANGLE360) zAngle -= math.ANGLE360;
}


void Camera::setAngle(float a)
{
	viewAngle = a;

	frustum.resetFrustum(viewAngle / zoom, viewDist);
	show();
}


void Camera::setViewDist(float d)
{
	viewDist = d;

	glFogf(GL_FOG_START,viewDist/8.0f);
	glFogf(GL_FOG_END,viewDist);

	frustum.resetFrustum(viewAngle / zoom, viewDist);
	show();
}


void Camera::setZoom(float z)
{
	zoom = z;

	frustum.resetFrustum(viewAngle / zoom, viewDist);
	show();
}


void Camera::show(void) const
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(viewAngle / zoom, (float)gOptions.RESX / (float)gOptions.RESY, 0.5f, viewDist);
	glMatrixMode(GL_MODELVIEW);

	glFogf(GL_FOG_START,viewDist/8.0f);
	glFogf(GL_FOG_END,viewDist);

	if (drawFog) {
		glFogf(GL_FOG_DENSITY, 1.0f);
		glEnable(GL_FOG);
	} else {
		glFogf(GL_FOG_DENSITY, 0);
		glDisable(GL_FOG);
	}
}


void Camera::renderWorld(void)
{
	if (!renderScene) return;

	// reset sun position
	glLightfv(GL_LIGHT0, GL_POSITION, level.getSkyBox().getSunPos().v);

	///// TEMP GOD MODE VIEW
	if (engine.cOptions.cameraView) {

		frustum.calcWorldPoints(p,zAngle,xAngle,yAngle);

		glRotatef(zAngle * math.getAngleInc(), 0,0,1);
		glRotatef(xAngle * math.getAngleInc(), 1,0,0);
		glRotatef(yAngle * math.getAngleInc(), 0,1,0);

		// Draw skybox without translating
//		if (engine.cOptions.rendersky) level.SkyBox->Render();

		glTranslatef(-p.x, -p.y, -p.z);

		if (engine.usePlayerCameraInDevMode) {
			// Draw terrain with player camera
			level.getTerrain().drawTerrain(camera.getCamera(engine.player->getViewCamID()));

			// reset sun position
			glLightfv(GL_LIGHT0, GL_POSITION, level.getSkyBox().getSunPos().v);

			// Render objects with player camera
			objectMgr.renderObjects(camera.getCamera(engine.player->getViewCamID()));
		} else {
			// Draw the terrain with this dev camera
			level.getTerrain().drawTerrain(*this);

			// reset sun position
			glLightfv(GL_LIGHT0, GL_POSITION, level.getSkyBox().getSunPos().v);

			// Draw objects with this dev camera
			objectMgr.renderObjects(*this);			
		}
				
		// Now draw the transparent triangles
		transTris.render();
		
		// draw player camera frustum
		camera.getCamera(engine.player->getViewCamID()).getFrustum().draw();

		glLoadIdentity();

	} else {

		// Update the frustum points
		frustum.calcWorldPoints(p,zAngle,xAngle,yAngle);

		// Rotate world into camera space
		glRotatef(zAngle * math.getAngleInc(), 0,0,1);
		glRotatef(xAngle * math.getAngleInc(), 1,0,0);
		glRotatef(yAngle * math.getAngleInc(), 0,1,0);

		// Draw skybox without translating
		if (engine.cOptions.renderSky) level.getSkyBox().render();

		// Translate world into camera space
		glTranslatef(-p.x, -p.y, -p.z);

		// Traverse the quadtree from the current camera
		level.getTerrain().drawTerrain(*this);	// Draw the terrain with this camera

		// reset sun position
		glLightfv(GL_LIGHT0, GL_POSITION, level.getSkyBox().getSunPos().v);

		// Draw objects here
		objectMgr.renderObjects(*this);

		// Now draw the transparent triangles
		transTris.render();

		glLoadIdentity();
	}
}


void Camera::setCameraSpaceMatrix(void) const // Translate world into camera space
{
	glRotatef(zAngle * math.getAngleInc(), 0,0,1);
	glRotatef(xAngle * math.getAngleInc(), 1,0,0);
	glRotatef(yAngle * math.getAngleInc(), 0,1,0);

	glTranslatef(-p.x, -p.y, -p.z);
}


Camera::Camera(const Vector3 &_p, int _xa, int _ya, int _za, float _va, float _vd, bool _fog)
{
	p = _p;
	
	xAngle = _xa;
	yAngle = _ya;
	zAngle = _za;

	viewAngle = _va;
	viewDist = _vd;
	zoom = 1;

	drawFog = _fog;
	renderScene = true;

	frustum.resetFrustum(_va, _vd);
}


Camera::~Camera()
{
}




