/* ----==== PLAYER.CPP ====---- */

#include <GL\glew.h>
#include "player.h"
#include "player_console.h"
#include "options.h"
#include "engine.h"
#include "camera.h"
#include "console.h"
#include "world.h"
#include "cameramanager.h"
#include "UTILITYCODE\glfont.h"
#include "UTILITYCODE\timer.h"
#include "UTILITYCODE\lookupmanager.h"
#include "UTILITYCODE\mousemanager.h"
#include "UTILITYCODE\keyboardmanager.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Player //////////


void Player::input(void)
{
	const float timeFix = timer.getTimeFix();	

	// Mouse-Look
	if (viewCamID == camera.getActiveID()) {	// if current camera is player cam
		yangle += (int)(mouse.getSensitiveDeltaX() / camera.getActiveCam().getZoom());
		xangle += (int)(mouse.getSensitiveDeltaY() / camera.getActiveCam().getZoom());
	} else {
		if (mouse.rightButtonDown()) {
			yangle += (int)(mouse.getSensitiveDeltaX());
			xangle += (int)(mouse.getSensitiveDeltaY());
		} else {
			camera.getActiveCam().setOrientationIncrement((int)mouse.getSensitiveDeltaY(), (int)mouse.getSensitiveDeltaX(), 0);
		}
	}
	
	if (yangle < 0) yangle += math.ANGLE360;
	else if (yangle >= math.ANGLE360) yangle -= math.ANGLE360;
	
	if (xangle < 0) xangle += math.ANGLE360;
	else if (xangle >= math.ANGLE360) xangle -= math.ANGLE360;
	if (xangle > math.ANGLE89 && xangle < math.ANGLE271) {
		if (abs(xangle - math.ANGLE89) < abs(xangle - math.ANGLE271)) xangle = math.ANGLE89;
		else xangle = math.ANGLE271;
	}

	// Leaning
	if (engine.keyState.LEANLEFT && engine.keyState.LEANRIGHT) {
		targetRollAngle = 0;
	} else if (engine.keyState.LEANLEFT) {
		targetRollAngle = -math.ANGLE10;
	} else if (engine.keyState.LEANRIGHT) {
		targetRollAngle = math.ANGLE10;
	} else {
		targetRollAngle = 0;
	}

	if (zangle < targetRollAngle) {
		zangle += (int)(timeFix * (float)math.ANGLE20);
		if (zangle > targetRollAngle) zangle = targetRollAngle;

	} else if (zangle > targetRollAngle) {
		zangle -= (int)(timeFix * (float)math.ANGLE20);
		if (zangle < targetRollAngle) zangle = targetRollAngle;
	}

	// Movement
	bool fdown = engine.keyState.FORWARD && !engine.keyState.BACKWARD;
	bool bdown = engine.keyState.BACKWARD && !engine.keyState.FORWARD;
	bool ldown = engine.keyState.STRAFELEFT && !engine.keyState.STRAFERIGHT;
	bool rdown = engine.keyState.STRAFERIGHT && !engine.keyState.STRAFELEFT;

	if (!jump && !engine.cOptions.cameraView) {
		float TopSpeed = maxspeed;

		bool move = false;

		if (engine.keyState.RUNTOGGLE) TopSpeed *= 0.4f;

		if (fdown && rdown) {
			move = true;
			moveyangle = yangle + math.ANGLE45 >= math.ANGLE360 ? yangle + math.ANGLE45 - math.ANGLE360 : yangle + math.ANGLE45;
		} else if (fdown && ldown) {
			move = true;
			moveyangle = yangle + math.ANGLE315 >= math.ANGLE360 ? yangle + math.ANGLE315 - math.ANGLE360 : yangle + math.ANGLE315;
		} else if (bdown && rdown) {
			move = true;
			moveyangle = yangle + math.ANGLE135 >= math.ANGLE360 ? yangle + math.ANGLE135 - math.ANGLE360 : yangle + math.ANGLE135;
		} else if (bdown && ldown) {
			move = true;
			moveyangle = yangle + math.ANGLE225 >= math.ANGLE360 ? yangle + math.ANGLE225 - math.ANGLE360 : yangle + math.ANGLE225;
		} else if (fdown) {
			move = true;
			moveyangle = yangle;
		} else if (bdown) {
			move = true;
			moveyangle = yangle + math.ANGLE180 >= math.ANGLE360 ? yangle + math.ANGLE180 - math.ANGLE360 : yangle + math.ANGLE180;
		} else if (rdown) {
			move = true;
			moveyangle = yangle + math.ANGLE90 >= math.ANGLE360 ? yangle + math.ANGLE90 - math.ANGLE360 : yangle + math.ANGLE90;
		} else if (ldown) {
			move = true;
			moveyangle = yangle + math.ANGLE270 >= math.ANGLE360 ? yangle + math.ANGLE270 - math.ANGLE360 : yangle + math.ANGLE270;
		}
		
		if (move) {
			runspeed += 100 * timeFix;
			if (runspeed > TopSpeed) runspeed = TopSpeed;
		} else {
			runspeed -= 200 * timeFix;
			if (runspeed < 0) runspeed = 0;
		}

		if (engine.keyState.JUMP && !jump && p.y == targety) {
			vertspeed = 14;
			jump = true;
		}

	} else if (engine.cOptions.cameraView) {

		// get look vector
		Vector3	lookV(0,0,-1);
		lookV.rot3D(camera.getActiveCam().getXAngle(), camera.getActiveCam().getYAngle(), camera.getActiveCam().getZAngle());
		lookV.normalize();

		Vector3 moveV(0,0,0);
		if (fdown && !bdown) { 
			moveV = lookV * ((engine.keyState.RUNTOGGLE ? maxspeed*0.5f : 1000.0f) * timeFix);
		} else if (bdown && !fdown) {
			moveV = -lookV * ((engine.keyState.RUNTOGGLE ? maxspeed*0.5f : 1000.0f) * timeFix);
		}

		if (ldown && !rdown) {
			lookV.assign(-1,0,0);
			lookV.rot3D(camera.getActiveCam().getXAngle(), camera.getActiveCam().getYAngle(), camera.getActiveCam().getZAngle());
			lookV.normalize();
			moveV += lookV * ((engine.keyState.RUNTOGGLE ? maxspeed*0.5f : 1000.0f) * timeFix);
		} else if (rdown && !ldown) {
			lookV.assign(1,0,0);
			lookV.rot3D(camera.getActiveCam().getXAngle(), camera.getActiveCam().getYAngle(), camera.getActiveCam().getZAngle());
			lookV.normalize();
			moveV += lookV * ((engine.keyState.RUNTOGGLE ? maxspeed*0.5f : 1000.0f) * timeFix);
		}

		if (engine.keyState.LEANLEFT && !engine.keyState.LEANRIGHT) {
			moveV.y -= (100.0f * timeFix);
		} else if (engine.keyState.LEANRIGHT && !engine.keyState.LEANLEFT) {
			moveV.y += (100.0f * timeFix);
		}

		// move the camera by the move vector
		camera.getActiveCam().setPosIncrement(moveV);
	}
}


void Player::update(void)
{
	float timeFix = timer.getTimeFix();
	newp = p;

/////

	if (runspeed != 0) {
		// Move Player
		if (!jump) {
			velsave.x = math.getSin(moveyangle) * runspeed;
			velsave.z = math.getCos(moveyangle) * runspeed;
		}
		newp.x = newp.x + velsave.x * timeFix;
		newp.z = newp.z - velsave.z * timeFix;

		const float mapfaredge = (float)(level.getTerrain().getSize()-1) * level.getTerrain().getHSpacing();
		if (newp.x < 0) newp.x = 0;
		else if (newp.x >= mapfaredge) newp.x = mapfaredge - 0.01f;
		if (newp.z < 0) newp.z = 0;
		else if (newp.z >= mapfaredge) newp.z = mapfaredge - 0.01f;
	}


	targety = level.getTerrain().getHeightBSpline(newp.x, newp.z/*, &normal, &concavity*/);

	if (jump) {
		p.y += vertspeed * timeFix;
		vertspeed += GRAVITY * timeFix;
		if (p.y <= targety) {
			jump = false;
			vertspeed = 0;
			p.y = targety;
		}
	} else p.y = targety;

	viewp.x = newp.x;
	p.x = viewp.x;
	
	viewp.z = newp.z;
	p.z = viewp.z;
	
	viewp.y = p.y + height;

	// Camera follows the player
	camera.getCamera(viewCamID).setPos(viewp);
	camera.getCamera(viewCamID).setOrientation(xangle,yangle, (zangle < 0 ? math.ANGLE360+zangle : zangle));
}


void Player::renderHUD(void) const
{
	///// Write some text /////
	if (drawHUD && !console.isVisible()) {
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_FOG);
		glDisable(GL_TEXTURE_2D);
		glDepthMask(GL_FALSE);

		const int fontSize = engine.font->getSize();
		int yPos = fontSize;

		if (!level.isLevelLoaded()) {
			
			glColor3f(1,1,1);
			engine.font->print(fontSize,yPos,"no level currently loaded");

		} else {
			
			glColor3f(1,1,1);
			engine.font->print(fontSize,yPos,"fps: %3.2f",timer.getFPS());
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"x: %5.2f  y: %5.2f  z: %5.2f", camera.getActiveCam().getP().x, camera.getActiveCam().getP().y, camera.getActiveCam().getP().z);
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"y angle: %i", camera.getActiveCam().getYAngle()/math.getAnglePrec());
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"x angle: %i", camera.getActiveCam().getXAngle()/math.getAnglePrec());
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"z angle: %i", camera.getActiveCam().getZAngle()/math.getAnglePrec());
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"draw mode (F): %s", engine.wireframe ? "wireframe" : "fill");
			yPos += fontSize;			
			engine.font->print(fontSize,yPos,"vertex caching (M): %s", engine.drawCached ? "on" : "off");
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"detail texturing (T): %s", engine.drawDetailLayer ? "on" : "off");
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"occlussion culling (O): %s", engine.actuallyocclude ? "on" : "off");
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"sunAngle: %0.3f", level.getSkyBox().getSunAngle());
			/*	
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"nx: %5.2f  ny: %5.2f  nz: %5.2f", normal.x, normal.y, normal.z);
			yPos += fontSize;
			engine.font->print(fontSize,yPos,"concavity: %0.2f", concavity);*/
			
			if (camera.getActiveCam().getZoom() != 1) {
			
				glColor3f(0.8f,0,0);

				engine.font->print(gOptions.RESX - (fontSize*4),fontSize,
							 "%0.1fX", camera.getActiveCam().getZoom());
			}
		}

		glDepthMask(GL_TRUE);
		if (!engine.cOptions.cameraView) glEnable(GL_FOG);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);
	}
}


Player::Player(const char *_n,float _x,float _y,float _z,int _xa,int _ya,int _za,float _maxspeed)
{
	height = 6;
	p.assign(_x, _y, _z);
	viewp = p;
	viewp.y = p.y + height;
	targety = p.y;
	xangle = _xa;
	yangle = moveyangle = _ya;
	zangle = _za;
	runspeed = vertspeed = 0;
	maxspeed = _maxspeed;
	jump = false;
	drawHUD = true;
	strncpy_s(name,_n,19);

	viewCamID = camera.createNewCamera(p,_xa,_ya,_za,VIEWANGLE,(float)gOptions.VIEWDIST,true);
	camera.setActiveID(viewCamID);

	cHandler = new Player_CAccess;
}


Player::~Player()
{
	delete cHandler;
}