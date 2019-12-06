/* ----==== TERRAINCHUNKCACHE.H ====---- */

#ifndef TERRAINCHUNKCACHE_H
#define TERRAINCHUNKCACHE_H

#include "UTILITYCODE/msgassert.h"
#include "MATHCODE/vector3.h"
#include "MATHCODE/matrix4x4.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Terrain;
class Matrix4x4;


class TerrainChunkCache {
	friend class Terrain;

	private:
		///// Attributes

		int				chunkSize;			// number of cells in a chunk (2^n)
		int				cellX, cellZ;		// these are stored so Terrain can make a matrix request using cell coordinates
										// of the terrain section rather than needing the chunk and cell offset
		// Matrix cached
		Matrix4x4		*chunkMatrices;

		// Height / normal cache Attributes
		float			*chunkHeights;		// cached height values
		Vector3			*chunkNormals;		// cached normals
		unsigned char	*cacheSkipBuffer;	// value is 0 when point has not been cached, 1 once it has
		int				cacheNumVerts;		// number of interpolated heights per axis in the chunk
		int				desiredLOD;			// the desired LOD level of the cached chunk
		int				attainedLOD;		// the actual max LOD of the cached chunk, always >= 0 and <= desiredLOD

		const Terrain	*terPtr;		// Pointer to owning Terrain

		///// Functions
		
		//------------------------------------------------------------------------------
		//	Caches interpolated height and normal points, returns true within time budget
		//------------------------------------------------------------------------------
		bool			cacheHeightsAndNormals(void);

	public:
		const Matrix4x4 &	getMatrix(int x, int z) const {
								msgAssert(z-cellZ >= 0 && z-cellZ < chunkSize && x-cellX >= 0 && x-cellX < chunkSize, "bad cell in cache matrix get");
								return chunkMatrices[(z-cellZ)*chunkSize+(x-cellX)];
							}
		
		const float			getHeight(int x, int z) const { return chunkHeights[z*cacheNumVerts+x];	}
		const Vector3		getNormal(int x, int z) const { return chunkNormals[z*cacheNumVerts+x]; }

		int					getCacheNumVerts() const { return cacheNumVerts; }
		int					getAttainedLOD() const { return attainedLOD; }
		bool				needsCaching() const { return attainedLOD < desiredLOD; }

		explicit TerrainChunkCache(const Terrain *ter, int chunkX, int chunkZ);
		~TerrainChunkCache();
};


#endif