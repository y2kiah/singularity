/* ----==== WORLD.CPP ====---- */

#include <GL\glew.h>
#include <cmath>
#include <fstream>
#include "world.h"
#include "world_console.h"
#include "engine.h"
#include "options.h"
#include "player.h"
#include "frustum.h"
#include "console.h"
#include "texturemanager.h"
#include "meshmanager.h"
#include "shadermanager.h"
#include "objectmanager.h"


/*------------------
---- STRUCTURES ----
------------------*/


////////// class Level //////////

bool Level::loadLevel(const std::string &filename)
{
	bool nameDone = false, heightDone = false, groundDone = false, groundMapDone = false;
	bool detailDone = false, skyDone = false, shaderDone = false;
	std::string inStr;

	levelLoaded = false;
	loadError = true;

	console.addLine(Color3f(1,1,1),"loading level \"%s\"",filename.c_str());

	std::ifstream inFile(filename.c_str());
	if (!inFile) {
		console.addLineErr("     file \"%s\" not found",filename.c_str());
		inFile.close();
		return false;
	}

	while (!inFile.eof()) {
		inStr.clear();
		std::getline(inFile,inStr);

		if (inStr == "[LEVEL NAME]") {
			nameDone = true;
			std::getline(inFile,levelName);
			
		} else if (inStr == "[HEIGHTMAP]") {
			heightDone = true;

			float hSpacing, vSpacing, texStretch, detTexStretch, detDist;

			std::getline(inFile,inStr);
			inFile >> hSpacing;
			inFile >> vSpacing;
			inFile >> texStretch;
			inFile >> detTexStretch;
			inFile >> detDist;

			terrain = new Terrain(gOptions.CHUNKSIZE, hSpacing, vSpacing, texStretch, detTexStretch, detDist);

			if (!terrain->loadRAW(std::string("data/level/")+inStr)) {
				console.addLineErr("     file \"%s\" not found", inStr.c_str());
				inFile.close();
				return false;
			}

		} else if (inStr == "[GROUND TEXTURES]") {
			if (!terrain) {
				console.addLineErr("     section [GROUND TEXTURES] must be after section [HEIGHTMAP]");
				inFile.close();
				return false;
			}			

			groundDone = true;
			
			int texCount = 0;
			inFile >> texCount;
			std::getline(inFile,inStr);

			terrain->groundTexID = new unsigned int[texCount];

			int texID;
			for (int t = 0; t < texCount; t++) {
				std::getline(inFile,inStr);

				texID = texture.loadFromFile(std::string("data/textures/")+inStr, GL_REPEAT, GL_MODULATE, true, true, true, false);
				if (texID == -1) {
					inFile.close();
					return false;
				} else {
					terrain->groundTexID[t] = texID;	// Store the global ID after loading the texture so
				}
			}

			// find number of lookups to use
			int numLookups = texCount / 4;
			if (texCount % 4 > 0) numLookups++;

			terrain->lookupTexID = new unsigned int[numLookups];

			for (int l = 0; l < numLookups; l++) {
				std::getline(inFile,inStr);
				
				texID = texture.loadFromFile(std::string("data/textures/")+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
				if (texID == -1) {
					inFile.close();
					return false;
				} else {
					terrain->lookupTexID[l] = texID;
				}
			}

		} else if (inStr == "[GROUND MAP TEXTURES]") {
			if (!terrain) {
				console.addLineErr("     section [GROUND MAP TEXTURES] must be after section [HEIGHTMAP]");
				inFile.close();
				return false;
			}			
			
			groundMapDone = true;
			
			int texCount = 0;
			inFile >> texCount;
			std::getline(inFile,inStr);

			terrain->layerTexID = new unsigned int[texCount];

			int texID;
			for (int t = 0; t < texCount; t++) {
				std::getline(inFile,inStr);

				texID = texture.loadFromFile(std::string("data/textures/")+inStr, GL_REPEAT, GL_MODULATE, true, true, true, false);
				if (texID == -1) {
					inFile.close();
					return false;
				} else {
					terrain->layerTexID[t] = texID;	// Store the global ID after loading the texture so
				}
			}

			// find number of lookups to use
			int numLookups = texCount / 4;
			if (texCount % 4 > 0) numLookups++;

			terrain->layerMapTexID = new unsigned int[numLookups];

			for (int l = 0; l < numLookups; l++) {
				std::getline(inFile,inStr);
				
				texID = texture.loadFromFile(std::string("data/textures/")+inStr, GL_CLAMP_TO_EDGE, GL_DECAL, false, true, false, false);
				if (texID == -1) {
					inFile.close();
					return false;
				} else {
					terrain->layerMapTexID[l] = texID;
				}
			}

		}/* else if (inStr == "[DETAIL OBJECT TEXTURES]") {
			if (!terrain) {
				console.addLineErr("     section [DETAIL OBJECT TEXTURES] must be after section [HEIGHTMAP]");
				inFile.close();
				return false;
			}			

			detailDone = true;
			
			int texcount = 0;
			inFile >> texcount;
			std::getline(inFile,inStr);

			detailTexture = new unsigned int[texcount];

			for (int c = 0; c < texcount; c++) {
				std::getline(inFile,inStr);
				
				int texID = texture.loadFromFile("data/texture/"+inStr, GL_CLAMP_TO_EDGE, GL_MODULATE, true, true, true);
				if (texID == -1) {
					inFile.close();
					return false;
				} else {
					detailTexture[c] = texID;
				}
			}

		}*/ else if (inStr == "[SKYBOX]") {
			skyDone = true;
			
			std::getline(inFile,inStr);
			skyBox = new Skybox;
			if (!skyBox->loadSkybox(std::string("data/level/")+inStr)) {
				console.addLineErr("     error loading skybox");
				inFile.close();
				return false;
			}

		} else if (inStr == "[SHADERS]") {
			if (!terrain) {
				console.addLineErr("     section [SHADERS] must be after section [HEIGHTMAP]");
				inFile.close();
				return false;
			}
			
			shaderDone = true;

			std::getline(inFile,inStr);
			int shaderID = shader.loadFromFile(std::string("data/shaders/"+inStr+".vert"), std::string("data/shaders/"+inStr+".frag"));
			if (shaderID == -1) {
				inFile.close();
				return false;
			} else {
				terrain->terShaderID = shaderID;	// store the terrain shader ID
			}

		}
	}

	inFile.close();


	// Check if all sections loaded
	if (!nameDone)		console.addLineErr("     [LEVEL NAME] section not found in \"%s\"",filename);
	if (!heightDone)	console.addLineErr("     [HEIGHTMAP] section not found in \"%s\"",filename);
	if (!groundDone)	console.addLineErr("     [GROUND TEXTURES] section not found in \"%s\"",filename);
	if (!groundMapDone)	console.addLineErr("     [GROUND MAP TEXTURES] section not found in \"%s\"",filename);
//	if (!detailDone)	console.addLineErr("     [DETAIL OBJECT TEXTURES] section not found in \"%s\"",filename);
	if (!skyDone)		console.addLineErr("     [SKYBOX] section not found in \"%s\"",filename);
	if (!shaderDone)	console.addLineErr("     [SHADERS] section not found in \"%s\"",filename);

	levelLoaded = nameDone && heightDone && groundDone && groundMapDone && /*detailDone &&*/ skyDone && shaderDone;
	loadError = !levelLoaded;
	return levelLoaded;
}


bool Level::unloadLevel(void)
{
	if (levelLoaded || loadError) {
		console.addLine("     level \"%s\" unloaded",levelName.c_str());

		delete terrain;
		delete skyBox;

		objectMgr.clearInstances();
		objectMgr.clearTypes();
		mesh.clear();
		texture.clear();
		
		terrain = 0;
		skyBox = 0;
		levelName.clear();
		levelLoaded = false;
		loadError = false;		

		return true;
	}	
	return false;
}


Level::Level() : Singleton<Level>(*this)
{
	levelLoaded = false;
	loadError = false;
	terrain = 0;
	skyBox = 0;
	levelName.clear();

	cHandler = new Level_CAccess;
}


Level::~Level()
{
	unloadLevel();
	delete cHandler;
}
