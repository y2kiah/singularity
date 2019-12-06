/* ----==== OPTIONS.CPP ====---- */

#include <fstream>
#include "options.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class GameOptions //////////

bool GameOptions::loadOptions(const std::string &filename)
{
	int boolset;
	std::string inStr;

	std::ifstream inFile;
	inFile.open(filename.c_str(), std::ios_base::in);
	if (!inFile.is_open()) return false;

	inFile.seekg(0);

	while (!inFile.eof()) {
		inStr.clear();
		std::getline(inFile,inStr);

		if (inStr == "[RESX]") {
			inFile >> RESX;
		} else if (inStr == "[RESY]") {
			inFile >> RESY;
		} else if (inStr == "[REFRESH]") {
			inFile >> REFRESH;
		} else if (inStr == "[COLORDEPTH]") {
			inFile >> COLORDEPTH;
		} else if (inStr == "[ZBUFFERDEPTH]") {
			inFile >> ZDEPTH;
		} else if (inStr == "[TEXTURESIZE]") {
			inFile >> TEXSIZE;
		} else if (inStr == "[FULLSCREEN]") {
			inFile >> boolset;
			FULLSCREEN = boolset == 1 ? true : false;
		} else if (inStr == "[VSYNCH]") {
			inFile >> boolset;
			VSYNCH = boolset == 1 ? true : false;
		} else if (inStr == "[VIEWDIST]") {
			inFile >> VIEWDIST;
		} else if (inStr == "[CHUNKSIZE]") {
			inFile >> CHUNKSIZE;
			if ((CHUNKSIZE < 1) || ((float)CHUNKSIZE / 2.0f != CHUNKSIZE / 2)) CHUNKSIZE = 8;
		} else if (inStr == "[CONSOLELINES]") {
			inFile >> CONSOLELINES;
		} else if (inStr == "[FONTSIZE]") {
			inFile >> FONTSIZE;
		} else if (inStr == "[MOUSESENSITIVITY]") {
			inFile >> MOUSESENSITIVITY;
		} else if (inStr == "[STARTLEVEL]") {
			inFile >> STARTLEVEL;
		}
	}

	inFile.close();
	return true;
}


GameOptions::GameOptions() : Singleton<GameOptions>(*this)
{
	RESX = 800;
	RESY = 600;
	REFRESH = 60;
	COLORDEPTH = 16;
	ZDEPTH = 16;
	TEXSIZE = 256;
	VIEWDIST = 600;
	CHUNKSIZE = 8;
	CONSOLELINES = 20;
	FONTSIZE = 12;
	MOUSESENSITIVITY = 10;
	STARTLEVEL = "";
	FULLSCREEN = false;
	VSYNCH = false;
}


////////// class GLExtensionOptions //////////

GlExtensionOptions::GlExtensionOptions() : Singleton<GlExtensionOptions>(*this)
{
	USE_GL_ARB_multitexture = false;
	USE_GL_ARB_texture_cube_map = false;
	USE_GL_ARB_texture_compression = false;
	USE_GL_EXT_texture_compression_s3tc = false;
	USE_WGL_EXT_swap_control = false;
	USE_GL_NV_fog_distance = false;
	USE_GL_NV_vertex_program3 = false;
	USE_GL_NV_fragment_program2 = false;
	MAX_TEXTURES_ARB = 0;
}
