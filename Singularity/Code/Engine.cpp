//	----==== ENGINE.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	This class owns all of the objects used for the engine
//	--------------------------------------------------------------------------------


#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <GL\glew.h>
#include <GL\wglew.h>
#include <cmath>
#include "engine.h"
#include "player.h"
#include "skybox.h"
#include "console.h"
#include "world.h"
#include "texturemanager.h"
#include "cameramanager.h"
#include "shadermanager.h"
#include "meshmanager.h"
#include "objectmanager.h"
#include "UTILITYCODE\glfont.h"
#include "UTILITYCODE\timer.h"
#include "UTILITYCODE\lookupmanager.h"
#include "UTILITYCODE\mousemanager.h"
#include "UTILITYCODE\keyboardmanager.h"

#include "frustum.h" ///// TEMP GOD MODE VIEW
#include "MATHCODE\vector4.h"

/*-----------------
---- VARIABLES ----
-----------------*/

extern HDC	hDC;


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Engine //////////


bool Engine::checkExtension(const char *extName) const
{
	if (glewIsSupported(extName) || wglewIsSupported(extName)) {
		console.addLine("GL extension found: %s", extName);
		return true;
	}

	console.addLineErr("GL extension not found: %s", extName);
	return false;
}


void Engine::initExtensions(void)
{
	if (!checkExtension("GL_ARB_vertex_shader")) {
		MessageBox(0,"GL_ARB_vertex_shader not supported", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
		return;
	}

	if (!checkExtension("GL_ARB_fragment_shader")) {
		MessageBox(0,"GL_ARB_fragment_shader not supported", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
		return;
	}

	if (!checkExtension("GL_ARB_shader_objects")) {
		MessageBox(0,"GL_ARB_shader_objects not supported", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
		return;
	}

	if (!checkExtension("GL_ARB_shading_language_100")) {
		MessageBox(0,"GL_ARB_shading_language_100", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
		return;
	}

	if (checkExtension("GL_ARB_multitexture")) {
		extOptions.USE_GL_ARB_multitexture = true;
		glGetIntegerv(GL_MAX_TEXTURE_UNITS_ARB, &extOptions.MAX_TEXTURES_ARB);
		console.addLine("     %i texture units found", extOptions.MAX_TEXTURES_ARB);
	} else {
		MessageBox(0,"ARB_multitexture not supported", "Error", MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
		return;
	}

	if (checkExtension("GL_ARB_texture_compression")) {
		extOptions.USE_GL_ARB_texture_compression = true;
	}

	if (checkExtension("GL_ARB_texture_cube_map")) {
		extOptions.USE_GL_ARB_texture_cube_map = true;
	}

	if (checkExtension("GL_EXT_texture_compression_s3tc")) {
		extOptions.USE_GL_EXT_texture_compression_s3tc = true;
	}

	if (checkExtension("GL_NV_fog_distance")) {
		extOptions.USE_GL_NV_fog_distance = true;
	}

	if (checkExtension("GL_NV_vertex_program3")) {
		extOptions.USE_GL_NV_vertex_program3 = true;
	}

	if (checkExtension("GL_NV_fragment_program2")) {
		extOptions.USE_GL_NV_fragment_program2 = true;
	}

	if (checkExtension("WGL_EXT_swap_control")) {
		extOptions.USE_WGL_EXT_swap_control = true;
	}
}


void Engine::initOpenGL(void)
{
	glewInit();
	initExtensions();

	///// Init OpenGL stuff /////

	glDrawBuffer(GL_BACK);

	glViewport(0,0,gOptions.RESX,gOptions.RESY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(VIEWANGLE, (float)gOptions.RESX / (float)gOptions.RESY, 1.0f, gOptions.VIEWDIST);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_SMOOTH);
	glCullFace(GL_BACK);
	glClearColor(0,0,0,1);
	glClearDepth(1.0);
	glDepthFunc(GL_LEQUAL);
	glDepthRange(0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // This is set up for distance fade
	glAlphaFunc(GL_NOTEQUAL,0);

	// Set up haze
//	const float FogColor[] = {0.641f,0.762f,0.953f,1};
	const float FogColor[] = {0.484f,0.535f,0.797f,1};
	glFogi(GL_FOG_MODE,GL_LINEAR);
	glFogfv(GL_FOG_COLOR,FogColor);
	glFogi(GL_FOG_START,gOptions.VIEWDIST/8);
	glFogi(GL_FOG_END,gOptions.VIEWDIST);
	glFogf(GL_FOG_DENSITY, 1.0f);
	if (extOptions.USE_GL_NV_fog_distance) glFogi(GL_FOG_DISTANCE_MODE_NV,GL_EYE_RADIAL_NV);

	if (true) {//screen.NICEGRAPHICS) { 
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
		glHint(GL_FOG_HINT, GL_NICEST);
		if (extOptions.USE_GL_ARB_texture_compression)
			glHint(GL_TEXTURE_COMPRESSION_HINT_ARB,GL_NICEST);
	} else {
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
		glHint(GL_POLYGON_SMOOTH_HINT,GL_FASTEST);
		glHint(GL_FOG_HINT, GL_FASTEST);
		if (extOptions.USE_GL_ARB_texture_compression)
			glHint(GL_TEXTURE_COMPRESSION_HINT_ARB,GL_FASTEST);
	}

	// Set up what to use
//	glEnable(GL_FOG);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);

	glDisable(GL_NORMALIZE);
	glDisable(GL_DITHER);
	glDisable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_LOGIC_OP);

	// Set up lighting	
	const GLfloat modelAmbient[] = {0.0f, 0.0f, 0.0f, 1.0f};
	const GLfloat sunCol[]  = {1.0f, 1.0f, 1.0f, 1.0f};
	const GLfloat sunAmbient[] = {0.4f, 0.4f, 0.4f, 1.0f};
	Vector4 sunPos(0.0f, 1.0f, 0.0f, 0.0f);
	sunPos.normalize();	
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_FALSE);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, modelAmbient);
	
	glLightfv(GL_LIGHT0, GL_POSITION, sunPos.v);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, sunCol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, sunAmbient);
	glEnable(GL_LIGHT0);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);	

	// Set up with vsynch on or off
	if (extOptions.USE_WGL_EXT_swap_control) {
		int set = 0;
		if (gOptions.VSYNCH) set = 1;
		
		if (wglSwapIntervalEXT(set) == FALSE) {
			console.addLineErr("wglSwapIntervalEXT function failed");
		}
	}
}


void Engine::setUpGame(void)
{
//	ShowCursor(false);
//	SetCursorPos(gOptions.RESX / 2,gOptions.RESY / 2);
	mouse.switchMode(MMODE_RECENTER);
	mouse.setVisible(FALSE);
	mouse.setSensitivity((float)gOptions.MOUSESENSITIVITY);

	// load font
	font = new GLFont(gOptions.RESY);
	if (!font->loadTrueTypeFont("Courier New",gOptions.FONTSIZE,FW_NORMAL,FALSE,FALSE,FALSE)) {
		MessageBox(0,"Error creating font","Error",MB_OK | MB_ICONERROR | MB_TOPMOST);
		PostQuitMessage(0);
	}	

	player = new Player("Kiah",0,0,0,0,0,0,200);
	
	levelInst = new Level;
	
	std::string lvlLoad = "levelload " + gOptions.STARTLEVEL;
	console.processLine("renderscene 0");
	console.processLine(lvlLoad.c_str());
	console.processLine("meshload church2.3ds,0");
	console.processLine("createobjecttype church,0,1");
	console.processLine("playerdrop 4400,4400");
	console.processLine("placeobject 0");
	console.processLine("playerdrop 4000,4000");
	console.processLine("renderscene 1");

	camera.getActiveCam().show();

	// Do some vertex array stuff
/*	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,level.Terrain->vertexmap);
	glTexCoordPointer(2,GL_FLOAT,0,level.Terrain->texcoordmap);
	glColorPointer(3,GL_UNSIGNED_BYTE,0,level.Terrain->shademap);*/
}


bool Engine::toggleButtonCheck(const short vk_code)
{
	if (kb.keyDown(vk_code)) {
		if (!toggleButton[vk_code]) {
			toggleButton[vk_code] = true;
			return true;
		}
	} else toggleButton[vk_code] = false;

	return false;
}

inline void Engine::resetKeyStates(void)
{
	keyState.FORWARD =
	keyState.BACKWARD =
	keyState.STRAFELEFT =
	keyState.STRAFERIGHT =
	keyState.CROUCH =
	keyState.JUMP =
	keyState.RUNTOGGLE = 
	keyState.LEANLEFT = 
	keyState.LEANRIGHT = false;
}

void Engine::getKeyStates(void)
{
	if (kb.keyDown('W')) keyState.FORWARD = true;
	if (kb.keyDown('S')) keyState.BACKWARD = true;

	if (kb.keyDown('A')) keyState.STRAFELEFT = true;
	if (kb.keyDown('D')) keyState.STRAFERIGHT = true;

	if (kb.keyDown('C')) keyState.CROUCH = true;
	if (kb.keyDown(VK_SPACE)) keyState.JUMP = true;

	if (kb.keyDown(VK_SHIFT)) keyState.RUNTOGGLE = true;
	
	if (kb.keyDown('Q')) keyState.LEANLEFT = true;
	if (kb.keyDown('E')) keyState.LEANRIGHT = true;
}

void Engine::playGame(void)
{
	// see if escape has been pressed for exit
	if (kb.keyDown(VK_ESCAPE)) exitEngine = true;

	const float timeFix = timer.getTimeFix();

	// Update mouse position once per frame
	mouse.updateMousePosition();

	// Toggle Console
	if (toggleButtonCheck(VkKeyScan('`'))) {
		std::string dummy;
		kb.appendAllInput(dummy); // clear the keyboard buffer of anything that was typed before console opens
		console.switchVisible();
	}

	// Toggle HUD display
	if (toggleButtonCheck(VK_TAB)) player->switchHUD();

	// Input to console or player movement
	resetKeyStates();
	if (console.isVisible()) console.input();
	else getKeyStates();

	if (!console.isVisible()) {
		// Zoom in
		if (toggleButtonCheck(VkKeyScan('=')))
			if (camera.getActiveCam().getZoom() < 16)
				camera.getActiveCam().zoomDouble();
		// Zoom out
		if (toggleButtonCheck(VkKeyScan('-')))
			if (camera.getActiveCam().getZoom() > 0.5f)
				camera.getActiveCam().zoomHalf();

		// Occlusion culling
		if (kb.keyTyped('O')) actuallyocclude = !actuallyocclude;
		// wireframe polygons
		if (kb.keyTyped('F')) wireframe = !wireframe;
		// terrain LOD
		if (kb.keyTyped(VK_SUBTRACT)) lod = (lod > 0) ? lod-1 : 0;
		if (kb.keyTyped(VK_ADD)) lod = (lod < 4) ? lod+1 : 4;
		// sunAngle
		float sunAngle = level.getSkyBox().getSunAngle();
		if (kb.keyDown(VK_OEM_4)) sunAngle -= 0.0555f * timeFix; sunAngle = (sunAngle <= 0.0f) ? 0.0f : sunAngle;
		if (kb.keyDown(VK_OEM_6)) sunAngle += 0.0555f * timeFix; sunAngle = (sunAngle >= 1.0f) ? 1.0f : sunAngle;
		// set sunPos
		int angleIndex = static_cast<int>(math.ANGLE180 * sunAngle);
		Vector4 sunPos(math.getCos(angleIndex), math.getSin(angleIndex), 0.0f, 0.0f);
		sunPos.normalize();
		level.getSkyBox().setSunAngle(sunAngle);
		level.getSkyBox().setSunPos(sunPos);
	
		// drawCached
		if (kb.keyTyped('M')) drawCached = !drawCached;
		// drawDetailLayer
		if (kb.keyTyped('T')) drawDetailLayer = !drawDetailLayer;
		if (kb.keyTyped('V') && engine.cOptions.cameraView) usePlayerCameraInDevMode = !usePlayerCameraInDevMode;

/*		if (kb.keyDown('Z')) {
			level.getTerrain().setVSpacing(level.getTerrain().getVSpacing() + (0.4f * timeFix));
			//level.getTerrain().rebuildNormalMap();
		} else if (kb.keyDown('X')) {
			level.getTerrain().setVSpacing(level.getTerrain().getVSpacing() - (0.4f * timeFix));
			//level.getTerrain().rebuildNormalMap();
		}*/
	}

	if (level.isLevelLoaded()) {
		// Update player
		player->input();
		player->update();
	}

	///// Begin rendering sequence
	wglSwapLayerBuffers(hDC,WGL_SWAP_MAIN_PLANE);
	//swapBuffers();

	if (!level.isLevelLoaded() || cOptions.cameraView || !cOptions.renderSky || !camera.getActiveCam().getRenderScene())
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); ///// TEMP GOD MODE VIEW
	else
		glClear(GL_DEPTH_BUFFER_BIT);

	if (level.isLevelLoaded()) {
		camera.renderActive();	// render the active camera
	}

	// Draw console
	if (console.isVisible()) console.render();
	else player->renderHUD();

	glFlush();
	///// End rendering sequence
}


Engine::Engine() : Singleton<Engine>(*this)
{
	// Load gOptions as first priority
	optionsInst = new GameOptions;
	gOptions.loadOptions("game.cfg");

	extOptionsInst = new GlExtensionOptions;

	consoleInst = new Console;
	console.registerBuiltInCommands();

	mouseInst = new MouseManager(gOptions.RESX, gOptions.RESY);
	keyboardInst = new KeyboardManager;
	lookupInst = new LookupManager(80);
	textureInst = new TextureManager;
	shaderInst = new ShaderManager;
	cameraInst = new CameraManager;
	meshInst = new MeshManager;
	objectMgrInst = new ObjectManager;
	player = 0;
	levelInst = 0;
	font = 0;

	for (int c = 0; c < 256; c++) toggleButton[c] = 0;

	///// TEMP, some instead of console variables
	actuallyocclude = true;
	wireframe = false;
	lod = 4;	
	drawCached = true;
	drawDetailLayer = true;
	usePlayerCameraInDevMode = false;
	exitEngine = false;
}


Engine::~Engine()
{	
	delete levelInst;
	delete player;
	delete font;
	delete cameraInst;
	delete shaderInst;
	delete textureInst;
	delete meshInst;
	delete objectMgrInst;
	delete lookupInst;
	delete keyboardInst;
	delete mouseInst;
	delete consoleInst;
	delete extOptionsInst;
	delete optionsInst;
}