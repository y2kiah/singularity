/* ----==== TERRAINLODHANDLER.H ====---- */

#ifndef TERRAINLODHANDLER_H
#define TERRAINLODHANDLER_H

#include "MATHCODE\vector3.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Terrain;


class TerrainLODHandler {
	private:
		const Terrain	*ptrTerrain;		// pointer to Terrain that owns this LOD Handler	
		int				numChunks;			// copied from terrain
		int				chunkSize;			// copied from terrain
		float			lodScale;			// = hSpacing / vSpacing

		float			*maxConcavityMap;	// pre-computed map of the maximum concavity on a patch
		int				*maxLODLevelMap;	// pre-computed map of max allowable LOD levels (numChunks x numChunks)
		int				*lodLevelMap;		// current frame's LOD levels (numChunks x numChunks)

	public:

		// reset to -1 so edges to uncomputed neighbors will be same lod as chunk (part of tri strip)
		void	clearFrameLODLevelMap(void)	{ for (int u = 0; u < numChunks*numChunks; ++u) lodLevelMap[u] = -1; }

		void	preComputeConcavity(void);
		void	computeFrameLODLevels(const Vector3 &camPos);

		float	getChunkConcavity(int chunkX, int chunkZ) const { return maxConcavityMap[chunkZ*numChunks+chunkX]; }
		
		int		getChunkLOD(int chunkX, int chunkZ) const { return lodLevelMap[chunkZ*numChunks+chunkX]; } // use with drawChunk
		int		getCachedChunkLOD(int chunkX, int chunkZ) const; // use with drawCachedChunk
		int		getMaxChunkLOD(int chunkX, int chunkZ) const { return maxLODLevelMap[chunkZ*numChunks+chunkX]; }

		explicit TerrainLODHandler(const Terrain *ter);
		~TerrainLODHandler();

};

#endif