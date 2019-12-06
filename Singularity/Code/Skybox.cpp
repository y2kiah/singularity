/* ----==== SKYBOX.CPP ====---- */

#include <GL\glew.h>
#include "skybox.h"
#include "skybox_console.h"
#include "engine.h"
#include "texturemanager.h"
#include "cameramanager.h"
#include "UTILITYCODE\lookupmanager.h"
#include "UTILITYCODE\timer.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// struct SkyLayer //////////

void SkyLayer::update(void)
{
	if (active) {
		const float timeFix = timer.getTimeFix();

		cu += uSpeed * timeFix;
		cv += vSpeed * timeFix;
			
		if (cu < 0.0f) cu += 1.0f;
		else if (cu >= 1.0f) cu -= 1.0f;
		
		if (cv < 0.0f) cv += 1.0f;
		else if (cv >= 1.0f) cv -= 1.0f;
	}
}


void SkyLayer::render(void) const
{
	if (active) {
		// stretch to viewdist*cos(45) to avoid cutting off corners for alpha blending
		const float viewDist = camera.getActiveCam().getViewDist() * math.getCos(math.ANGLE45);
		const float height = viewDist * 0.05f;	// 1/20th, ratio looks good for cloud height
		const float tRep = 5.0f;
		const float tRepHalf = tRep * 0.5f;

		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);

		glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID).getGLTexID());
		glBegin(GL_TRIANGLE_FAN);
			// center of triangle fan, alpha 1
			glColor4f(1,1,1,1);
			glTexCoord2f(cu+tRepHalf, cv+tRepHalf);
			glVertex3f(0,height,0);

			// corner points, alpha 0
			glColor4f(1,1,1,0);
			glTexCoord2f(cu, cv);
			glVertex3f(-viewDist,height,-viewDist);

			glTexCoord2f(cu+tRepHalf, cv);
			glVertex3f(0,height,-viewDist);

			glTexCoord2f(cu+tRep, cv);
			glVertex3f(viewDist,height,-viewDist);

			glTexCoord2f(cu+tRep, cv+tRepHalf);
			glVertex3f(viewDist,height,0);

			glTexCoord2f(cu+tRep, cv+tRep);
			glVertex3f(viewDist,height,viewDist);

			glTexCoord2f(cu+tRepHalf, cv+tRep);
			glVertex3f(0,height,viewDist);

			glTexCoord2f(cu, cv+tRep);
			glVertex3f(-viewDist,height,viewDist);

			glTexCoord2f(cu, cv+tRepHalf);
			glVertex3f(-viewDist,height,0);

			glTexCoord2f(cu, cv);
			glVertex3f(-viewDist,height,-viewDist);
		glEnd();

		glColor4f(1,1,1,1);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
	}
}


void SkyLayer::setSpeed(float us, float vs)
{
	uSpeed = us;
	vSpeed = vs;
	cu = cv = 0;
}


SkyLayer::SkyLayer(int texID, float us, float vs) :
	active(true),
	textureID(texID),
	uSpeed(us),
	vSpeed(vs),
	cu(0), cv(0) {
}


////////// class Skybox //////////

void Skybox::render(void)
{
	glDisable(GL_FOG);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDepthMask(GL_FALSE);
//	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	glColor3f(1,1,1);

	// Front
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[0]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(1,0);
		glVertex3i(100,-100,-100);

		glTexCoord2i(1,1);
		glVertex3i(100,100,-100);

		glTexCoord2i(0,1);
		glVertex3i(-100,100,-100);

		glTexCoord2i(0,0);
		glVertex3i(-100,-100,-100);
	glEnd();

	// Left
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[1]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(1,0);
		glVertex3i(-100,-100,-100);

		glTexCoord2i(1,1);
		glVertex3i(-100,100,-100);

		glTexCoord2i(0,1);
		glVertex3i(-100,100,100);

		glTexCoord2i(0,0);
		glVertex3i(-100,-100,100);
	glEnd();

	// Back
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[2]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(1,0);
		glVertex3i(-100,-100,100);

		glTexCoord2i(1,1);
		glVertex3i(-100,100,100);

		glTexCoord2i(0,1);
		glVertex3i(100,100,100);

		glTexCoord2i(0,0);
		glVertex3i(100,-100,100);
	glEnd();

	// Right
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[3]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(1,0);
		glVertex3i(100,-100,100);

		glTexCoord2i(1,1);
		glVertex3i(100,100,100);

		glTexCoord2i(0,1);
		glVertex3i(100,100,-100);

		glTexCoord2i(0,0);
		glVertex3i(100,-100,-100);
	glEnd();

	// Top
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[4]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(0,0);
		glVertex3i(-100,100,-100);

		glTexCoord2i(1,0);
		glVertex3i(100,100,-100);

		glTexCoord2i(1,1);
		glVertex3i(100,100,100);

		glTexCoord2i(0,1);
		glVertex3i(-100,100,100);
	glEnd();

	// Bottom
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(textureID[5]).getGLTexID());
	glBegin(GL_QUADS);
		glTexCoord2i(0,1);
		glVertex3i(-100,-100,100);

		glTexCoord2i(1,1);
		glVertex3i(100,-100,100);

		glTexCoord2i(1,0);
		glVertex3i(100,-100,-100);

		glTexCoord2i(0,0);
		glVertex3i(-100,-100,-100);
	glEnd();

//	glDisable(GL_TEXTURE_CUBE_MAP_ARB);

	// Sky layers
	glEnable(GL_FOG);
	
	for (int l = 0; l < layer.size(); l++) {
		layer[l]->update();
		layer[l]->render();
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDepthMask(GL_TRUE);
}

bool Skybox::loadSkybox(const std::string &filename)
{
	std::string inStr;

	std::ifstream inFile(filename.c_str());
	if (!inFile) {
		console.addLineErr("     file \"%s\" not found", filename.c_str());
		return false;
	}

	std::string dir = "data/textures/";
	
	while (!inFile.eof()) {
		
		std::getline(inFile,inStr);

		if (inStr == "[PLUSZ]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[0] = texID;

		} else if (inStr == "[MINUSZ]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[2] = texID;

		} else if (inStr == "[MINUSX]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[3] = texID;

		} else if (inStr == "[PLUSX]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[1] = texID;

		} else if (inStr == "[PLUSY]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[4] = texID;

		} else if (inStr == "[MINUSY]") {
			std::getline(inFile,inStr);

			int texID = texture.loadFromFile(dir+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
			if (texID == -1) {
				inFile.close();
				return false;
			} else textureID[5] = texID;

		} else if (inStr == "[LAYERS]") {
			int numLayers = 0;
			
			inFile >> numLayers;
			layer.reserve(numLayers);

			for (int c = 0; c < numLayers; c++) {
				float uSpeed = 0, vSpeed = 0;
				
				inFile >> uSpeed >> vSpeed;
				inFile >> inStr;
				//std::getline(inFile,inStr);
				
				int texID = texture.loadFromFile(dir+inStr, GL_REPEAT, GL_MODULATE, false, true, true, false);
				if (texID == -1) {
					inFile.close();
					return false;
				}
				
				layer.push_back(new SkyLayer(texID,uSpeed,vSpeed));

				console.addLine("     added SkyLayer %i: texID %i: uSpeed %0.2f: vSpeed %0.2f",c,texID,uSpeed,vSpeed);
			}
		}
	}

	inFile.close();

	return true;
}

Skybox::Skybox()
{
	for (int c = 0; c < 6; ++c) textureID[c] = -1;
	
	sunPos.assign(0.0f, 1.0f, 0.0f, 0.0f);
	sunPos.normalize();

	cHandler = new Skybox_CAccess;
}

Skybox::~Skybox()
{
	for (int l = 0; l < layer.size(); ++l) { delete layer[l]; }
	layer.clear();

	delete cHandler;
}