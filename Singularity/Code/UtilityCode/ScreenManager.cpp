//	----==== SCREENMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			6/04
//	Description:	Organizes several display options into a manager
//	---------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL\glew.h>
#include "screenmanager.h"


/*-----------------
---- VARIABLES ----
-----------------*/

extern HDC		hDC;

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class ScreenManager //////////

/*void ScreenManager::initOpenGL(void)
{
	///// Init OpenGL stuff /////

	glViewport(0,0,RESX,RESY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	gluPerspective(VIEWANGLE, (float)RESX / (float)RESY, 1.0f, VIEWDIST);
	glOrtho(-10,10,-10,10,1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glClearColor(0,0,0,1);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // This is set up for the splat textures
	glAlphaFunc(GL_NOTEQUAL,0);

	if (NICEGRAPHICS) { 
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
		glHint(GL_FOG_HINT, GL_NICEST);
	} else {
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST);
		glHint(GL_FOG_HINT, GL_FASTEST);
	}

	// Set up what to use
	glEnable(GL_CULL_FACE);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_FOG);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_NORMALIZE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LOGIC_OP);

	// set up lighting
	const float ambient[4] = {0.4f,0.4f,0.4f,1};
	const float lightcol[4] = {1,1,1,1};
	const float direction[4] = {1,1,1,0};
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightcol);
	glLightfv(GL_LIGHT0, GL_POSITION, direction);
	glEnable(GL_LIGHT0);

	glEnable(GL_COLOR_MATERIAL);	
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

	glEnable(GL_LIGHTING);
}*/


ScreenManager::ScreenManager(int resX, int resY, int refresh, int colorDepth, int zDepth,
							 bool niceGraphics, bool fullScreen) : Singleton<ScreenManager>(*this)
{		
	RESX = resX;
	RESY = resY;
	REFRESH = refresh;
	COLORDEPTH = colorDepth;
	ZDEPTH = zDepth;
	NICEGRAPHICS = niceGraphics;
	FULLSCREEN = fullScreen;
}


ScreenManager::ScreenManager() : Singleton<ScreenManager>(*this)
{
	RESX = 1024;
	RESY = 768;
	REFRESH = 60;
	COLORDEPTH = 32;
	ZDEPTH = 24;
	NICEGRAPHICS = true;
	FULLSCREEN = false;
//	VIEWANGLE = 60;
//	VIEWDIST = 1000;
}
