/* ----==== TERRAIN.H ====---- */

#ifndef TERRAIN_H
#define TERRAIN_H

#include "terrainchunkcache.h"
#include "camera.h"
#include "console.h"


/*------------------
---- STRUCTURES ----
------------------*/

class TerrainOccluder;
class TerrainLODHandler;
class TerrainShadow;
class DetailManager;
class Matrix4x4;


class Terrain {
	friend class Level;	// for setting variables while loading
	friend class TerrainOccluder;
	friend class TerrainLODHandler;
	friend class TerrainChunkCache;
	friend class TerrainShadow;

	private:
		int				size;				// height points on one axis (2^n + 1)
		int				numChunks;			// = (size-1) / chunkSize
		int				chunkSize;			// size of smallest terrain chunk (2^n)
		int				chunkNumVerts;		// = chunkSize+1;
		float			chunkLength;		// = chunkSize * hSpacing;
		float			mapLength;			// = (size-1) * hSpacing;
		float			invMapLength;		// inverse
		float			hSpacing;			// in feet
		float			vSpacing;			// in feet
		float			maxTerrainHeight;	// maximum elevation
		float			texStretch;			// how many texture lengths will stretch over one terrain cell - hSpacing mod texStretch = 0
		float			detTexStretch;		// same as above for detailed texture layer - hSpacing mod detTexStretch = 0
		float			texMult;			// use for texture coords (= texStretch / hSpacing)
		float			texMultDet;			// use for detail texture coords (= detTexStretch / hSpacing)
		float			detailDist;			// distance that the detail layer will extend from the camera

		unsigned char	*heightMap;			// height point table

		unsigned int	*groundTexID;		// arrays for storing texture IDs
		unsigned int	*lookupTexID;
		unsigned int	*layerTexID;
		unsigned int	*layerMapTexID;

		unsigned int	terShaderID;		// base terrain shader ID
		unsigned int	detShaderID;		// detail layer shader ID

		TerrainOccluder		*tOccluder;		// terrain occluder used for drawing
		TerrainLODHandler	*tLODHandler;	// terrain level of detail handler
		TerrainShadow		*tShadow;		// terrain shadow handler
		DetailManager		*dManager;		// handles drawing of terrain detail layers
		
		///// Terrain caching variables
		TerrainChunkCache	**chunkCache;			// stores precomputed matrices in a cache for fast rendering
		int					cacheBoxMinChunkX;		// location of the cache box in chunk coordinates
		int					cacheBoxMinChunkZ;		// Chunks are cached so location is stored as upperleft-most chunk
		int					cacheBoxMaxChunkX;		// maximum chunk to cache
		int					cacheBoxMaxChunkZ;
		float				cacheMaxTimeInSec;		// the maximum amount of time to spend each frame caching patch data
		_int64				cacheTimeStarted;		// caching time started each frame
		
		///// Functions
		const Matrix4x4 &	getCachedMatrixForCell(int cellX, int cellZ) const;

		void				cacheChunks(const Camera &cam);	// perform the caching routine
		void				forceReCache(void);				// next frame will recache all chunks (even visible)


		bool	checkBlendDistance(	int cellX, int cellZ, float posX, float posZ,
									float fadeStartDistSq, const Camera &cam) const;
		void	drawChunk(int cellX, int cellZ, const Camera &cam) const;
		void	drawCachedChunk(int cellX, int cellZ, const Camera &cam) const;

//		void	drawDetailLayer(const Camera &cam) const;
		
		///// BSpline patch functions
		void	getBSplineControlPointsFromHeightmap(int cellX, int cellZ, float *buffer) const;
		void	drawBSplineChunk(int cellX, int cellZ, int lod, int lodMinusX, int lodMinusZ, int lodPlusX, int lodPlusZ) const;
		void	drawCachedBSplineChunk(int cellX, int cellZ, int lod, int maxLOD, int *edgeLODs, int *edgeMaxLODs) const;

		void	drawBSplineChunkOptimized(int cellX, int cellZ, int lodMinusX, int lodMinusZ, int lodPlusX, int lodPlusZ) const;
		void	drawBSplineChunkEdge(int cellX, int cellZ, int innerLod, int outerLod, float innerDeltaT, float innerSpacing,
										unsigned char edge, unsigned char drawNearCorner, unsigned char drawFarCorner) const;


		///// Old terrain functions		
//		Vector3			*vNormalMap;		// vertex normals
//		unsigned short	*indexBuffer;		// index buffer updated dynamically for each chunk drawn
//		unsigned int	vertexBuffer;		// these are the VBO IDs
//		unsigned int	normalBuffer;
//		unsigned int	texBuffer0;
//		unsigned int	texBuffer1;
//		int				indexBufferSize;	// size of the index buffer

//		void			drawChunkVBO(int cellX, int cellZ, const Camera &cam) const;
//		void			buildNormalMap(void);
//		void			fillIndexBuffer(int xOffset, int zOffset);
//		void			buildVertexBuffers(void);
//		void			destroyVertexBuffers(void);
//		bool			createHorizonTexture(void);

//		void			enableVertexBuffers(void) const;
//		void			disableVertexBuffers(void) const;

	public:
//		float			getHeight(float _x, float _z) const;
		///// temp
		void			setVSpacing(float set) { vSpacing = (set >= 0.1f ? set : 0.1f); forceReCache(); }
		void			setHSpacing(float set) { hSpacing = (set >= 0.1f ? set : 0.1f); forceReCache(); }
		/////

		///// TEMP lighting
		Vector3				lightDir; // pointing from origin to sun
		void				createHorizonMap(int texSize, const std::string &filename);
		/////

		void			setCacheMaxTimeInSec(float t) { cacheMaxTimeInSec = t; }

		int				getSize() const { return size; }
		float			getHSpacing() const { return hSpacing; }
		float			getVSpacing() const { return vSpacing; }
		float			getHeightBSpline(float _x, float _z/*, Vector3 *normal, float *concavity*/) const;	
		Vector3			getNormalBSpline(float _x, float _z) const;

		void			drawTerrain(const Camera &cam);
		bool			loadRAW(const std::string &filename);

		explicit Terrain(	int _chunkSize, float _hSpacing, float _vSpacing,
							float _texStretch, float _detTexStretch, float _detailDist);
		~Terrain();
};


#endif