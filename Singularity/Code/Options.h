/* ----==== OPTIONS.H ====---- */

#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include "UTILITYCODE\singleton.h"

/*---------------
---- DEFINES ----
---------------*/

#define gOptions		GameOptions::instance()			// used to access GameOptions instance globally
#define extOptions		GlExtensionOptions::instance()	// used to access GLExtensionOptions instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class GameOptions : public Singleton<GameOptions> {	
	public:
		int			RESX;
		int			RESY;
		int			REFRESH;
		int			COLORDEPTH;
		int			ZDEPTH;
		int			TEXSIZE;
		int			VIEWDIST;
		int			CHUNKSIZE;	// must be 2^n and > 1
		int			CONSOLELINES;
		int			FONTSIZE;
		int			MOUSESENSITIVITY;
		std::string	STARTLEVEL;
		bool		FULLSCREEN;
		bool		VSYNCH;

		bool loadOptions(const std::string &filename);

		explicit GameOptions();
};

class GlExtensionOptions : public Singleton<GlExtensionOptions> {
	public:
		bool	USE_GL_ARB_multitexture;
		bool	USE_GL_ARB_texture_cube_map;
		bool	USE_GL_ARB_texture_compression;
		bool	USE_GL_EXT_texture_compression_s3tc;
		bool	USE_GL_NV_fog_distance;
		bool	USE_GL_NV_vertex_program3;
		bool	USE_GL_NV_fragment_program2;
		bool	USE_WGL_EXT_swap_control;
		int		MAX_TEXTURES_ARB;

		explicit GlExtensionOptions();
};

struct ConsoleOptions {
	bool cameraView;
	bool renderSky;	

	explicit ConsoleOptions() { cameraView = false; renderSky = true; }
};

#endif