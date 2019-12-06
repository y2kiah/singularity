/* ----==== TERRAINLODHANDLER.CPP ====---- */

#include "terrainlodhandler.h"
#include "terrain.h"
#include "MATHCODE/spline.h"

#include "engine.h" // temp for LOD

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class TerrainLODHandler //////////


void TerrainLODHandler::preComputeConcavity(void)
{
	CubicBSpline::setSpacing(ptrTerrain->getHSpacing(), ptrTerrain->getVSpacing());  
	float hpBuffer[16];

	for (int z = 0; z < numChunks; ++z) {
		for (int x = 0; x < numChunks; ++x) {
			float maxConcavity = 0;
			int cellZ = z * chunkSize;
			for (int cZ = 0; cZ < chunkSize; ++cZ) {
				int cellX = x * chunkSize;
				for (int cX = 0; cX < chunkSize; ++cX) {
					ptrTerrain->getBSplineControlPointsFromHeightmap(cellX, cellZ, hpBuffer);
					CubicBSpline::preCalcMiddleMatrix(hpBuffer); 
					
					float concavity = CubicBSpline::calcConcavityOnPatchMatrix(0, 0, false);
					if (concavity > maxConcavity) maxConcavity = concavity;

					if (cX == chunkSize-1) {	// last column
						concavity = CubicBSpline::calcConcavityOnPatchMatrix(1.0f, 0, false);
						if (concavity > maxConcavity) maxConcavity = concavity;
					}
					if (cZ == chunkSize-1) {	// last row
						concavity = CubicBSpline::calcConcavityOnPatchMatrix(0, 1.0f, false);
						if (concavity > maxConcavity) maxConcavity = concavity;
					}

					++cellX;
				}
				++cellZ;
			}

			maxConcavityMap[z*numChunks+x] = maxConcavity;

			if (maxConcavity < EPSILON) { // if it's 0
				maxLODLevelMap[z*numChunks+x] = 0;
			} else if (maxConcavity < 4.0f) {
				maxLODLevelMap[z*numChunks+x] = 1;
			} else if (maxConcavity < 10.0f) {
				maxLODLevelMap[z*numChunks+x] = 2;
			} else if (maxConcavity < 28.0f) {
				maxLODLevelMap[z*numChunks+x] = 3;
			} else {
				maxLODLevelMap[z*numChunks+x] = 4;
			}

/*			if (maxConcavity >= 8.0f) {
				minLODLevelMap[z*numChunks+x] = 1;
			} else if (maxConcavity >= 24.0f) {
				minLODLevelMap[z*numChunks+x] = 2;
			} else if (maxConcavity >= 40.0f) {
				minLODLevelMap[z*numChunks+x] = 3;
			} else {
				minLODLevelMap[z*numChunks+x] = 0;
			}*/
		}
	}
}


// for cached chunks, this should be called after caching for the frame when cacheBox... variables are current to frame
void TerrainLODHandler::computeFrameLODLevels(const Vector3 &camPos)
{
	int minChunkX = ptrTerrain->cacheBoxMinChunkX;
	int minChunkZ = ptrTerrain->cacheBoxMinChunkZ;
	int maxChunkX = ptrTerrain->cacheBoxMaxChunkX;
	int maxChunkZ = ptrTerrain->cacheBoxMaxChunkZ;

	float chunkLength = ptrTerrain->chunkLength;
	int numChunks = ptrTerrain->numChunks;
	int chunkSize = ptrTerrain->chunkSize;

	for (int z = minChunkZ; z <= maxChunkZ; ++z) {
		for (int x = minChunkX; x <= maxChunkX; ++x) {
			
			// find distance squared to center of chunk
			float chunkPosX = x * chunkLength;
			float chunkPosZ = z * chunkLength;
			float chunkHeight = ptrTerrain->heightMap[((z*chunkSize+(chunkSize>>1))*numChunks)+(x*chunkSize+(chunkSize>>1))]*ptrTerrain->hSpacing;
			Vector3 chunkCenter(chunkPosX + (chunkLength*0.5f), chunkHeight, chunkPosZ + (chunkLength*0.5f));
			float distSqToChunk = camPos.distSquared(chunkCenter);

			int lod = 0;
			if (distSqToChunk <= 570*570) {
				lod = 4;
			} else if (distSqToChunk <= 600*600) {
				lod = 3;
			} else if (distSqToChunk <= 1200*1200) {
				lod = 2;
			} else if (distSqToChunk <= 2400*2400) {
				lod = 1;
			}

			if (lod > maxLODLevelMap[z*numChunks+x]) lod = maxLODLevelMap[z*numChunks+x];
			if (lod > engine.lod) lod = engine.lod;

			lodLevelMap[z*numChunks+x] = lod;			
		}
	}
}


//------------------------------------------------------------------------------
//	returns the lesser of cached chunk's attained LOD or frame selected LOD
//------------------------------------------------------------------------------
int TerrainLODHandler::getCachedChunkLOD(int chunkX, int chunkZ) const
{
	int attainedLOD = ptrTerrain->chunkCache[chunkZ*numChunks+chunkX]->getAttainedLOD();

	return attainedLOD < lodLevelMap[chunkZ*numChunks+chunkX] ? attainedLOD : lodLevelMap[chunkZ*numChunks+chunkX];
}


TerrainLODHandler::TerrainLODHandler(const Terrain *ter) : ptrTerrain(ter)
{	
	numChunks = ter->numChunks;
	chunkSize = ter->chunkSize;

	lodScale = (ter->hSpacing == 0.0f) ? ter->vSpacing : ter->vSpacing / ter->hSpacing;

	maxConcavityMap = new float[numChunks*numChunks];
	maxLODLevelMap = new int[numChunks*numChunks];
	lodLevelMap = new int[numChunks*numChunks];

	preComputeConcavity();
}


TerrainLODHandler::~TerrainLODHandler()
{
	delete [] maxConcavityMap;
	delete [] maxLODLevelMap;
	delete [] lodLevelMap;
}

