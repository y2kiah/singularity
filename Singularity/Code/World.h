/* ----==== WORLD.H ====---- */

#ifndef WORLD_H
#define WORLD_H

#include <string>
#include "terrain.h"
#include "skybox.h"
#include "UTILITYCODE\singleton.h"
#include "UTILITYCODE\color.h"
#include "MATHCODE\vector3.h"

/*---------------
---- DEFINES ----
---------------*/

#define level		Level::instance()	// used to access the Level instance globally

/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class Level_CAccess;


class Level : public Singleton<Level> {
	friend class Level_CAccess;
	friend class Skybox_CAccess;
	
	private:
		std::string	levelName;
		bool		levelLoaded;
		bool		loadError;

		Terrain		*terrain;
		Skybox		*skyBox;

		bool		loadLevel(const std::string &filename);
		bool		unloadLevel(void);

		CHandler	*cHandler;

	public:		
		
		bool		isLevelLoaded() const { return levelLoaded; }
		Terrain	&	getTerrain() const { return *terrain; }
		Skybox &	getSkyBox() const { return *skyBox; }

		explicit Level();
		~Level();
};

#endif