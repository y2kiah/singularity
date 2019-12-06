/* ----==== TERRAINSHADOW.H ====---- */

#ifndef TERRAINSHADOW_H
#define TERRAINSHADOW_H

#include <string>

/*------------------
---- STRUCTURES ----
------------------*/

class Terrain;
class Vector3;
class Color3uc;

class TerrainShadow {
	private:

		/*const*/ Terrain	&terRef;		
		
		///// ShadowMap
		// Attributes
		unsigned int	shadowMapTexID;
		bool			shadowMapLoaded;

		// Methods
		void			computeShadowMap(const Vector3 *dirToSun, float ambientSun, float ambientShade,
											int texSize, Color3uc *shadowMap, int index);
		void			blur(Color3uc *source, int width, int height, int radius);
		bool			saveShadowMapToFile(const std::string &filename, int texSize, Color3uc *shadowMap);

		///// NormalMap
		// Attributes
		unsigned int	normalMapTexID;
		bool			normalMapLoaded;

		// Methods
		void			createNormalMap(int texSize);

		///// HorizonMap
		// Attributes
		unsigned int	horizonMapTexID;
		bool			horizonMapLoaded;

		// Methods


	public:
		// Accessors
		unsigned int	getShadowMapTexID() const { return shadowMapTexID; }
		unsigned int	getNormalMapTexID() const { return normalMapTexID; }
		unsigned int	getHorizonMapTexID() const { return horizonMapTexID; }
		
		bool			isShadowMapLoaded() const { return shadowMapLoaded; }
		bool			isNormalMapLoaded() const { return normalMapLoaded; }
		bool			isHorizonMapLoaded() const { return horizonMapLoaded; }

		bool			createShadowMap(const Vector3 *dirToSun, float ambientSun, float ambientShade,
										int texSize, const std::string &filename);
		bool			loadFromRAW(const std::string &filename);

		///// HORIZON MAP STUFF TEMP PUBLIC /////
		bool			createHorizonMap(int texSize, const std::string &filename);
		bool			loadHorizonMap(const std::string &filename);			
		/////////////////////////////

		explicit TerrainShadow(/*const */Terrain *ter);
		~TerrainShadow() {}
};


#endif