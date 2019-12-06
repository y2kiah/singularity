/* ----==== TERRAINCHUNKCACHE.CPP ====---- */

#include "terrainchunkcache.h"
#include "terrainlodhandler.h"
#include "terrain.h"
#include "MATHCODE\spline.h"
#include "UTILITYCODE\timer.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class TerrainChunkCache //////////


//------------------------------------------------------------------------------
//	Caches interpolated height points and attains a certain LOD per frame based
//	on time budget set in Terrain. If desiredLOD not attained in one pass,
//	subsequent calls are made to fill in gaps.
//------------------------------------------------------------------------------
/*
void TerrainChunkCache::cacheHeightsAndNormals(void)
{
	int numUnitsPerCell = 1 << desiredLOD;

	// get delta t (0.0 <= t <= 1.0) for surface sampling
	float deltaT = 1.0f / (float)numUnitsPerCell;

	// find height points

	int iz = 0;
	for (int cv = 0; cv < chunkSize; ++cv) {	// for each patch within the chunk
		
		float t = 0;
		int endZ = (cv < chunkSize-1) ? numUnitsPerCell : numUnitsPerCell+1; // this is so the last row of vertices will be included
		for (int uz = 0; uz < endZ; ++uz) {	// for each vertex within a patch
			
			int ix = 0;	
			for (int ch = 0; ch < chunkSize; ++ch) {

				CubicBSpline::setMiddleMatrixPtr(chunkMatrices[cv * chunkSize + ch]);				
							
				float s = 0;
				int endX = (ch < chunkSize-1) ? numUnitsPerCell : numUnitsPerCell+1;
				for (int ux = 0; ux < endX; ++ux) {
					chunkHeights[iz*cacheNumVerts + ix] = CubicBSpline::calcHeightOnPatchMatrix(s, t, true);
					chunkNormals[iz*cacheNumVerts + ix] = CubicBSpline::calcNormalOnPatchMatrix(s, t, true);
				
					++ix;
					s += deltaT;
				}
			}

			++iz;
			t += deltaT;				
		}
	}
}
*/


//------------------------------------------------------------------------------
//	Caches interpolated height points and attains a certain LOD per frame based
//	on time budget set in Terrain. If desiredLOD not attained in one pass,
//	subsequent calls are made to fill in gaps.
//	Returns false if time exceeded budget, true otherwise
//------------------------------------------------------------------------------
bool TerrainChunkCache::cacheHeightsAndNormals(void)
{
	bool withinTimeBudget = true;
	const Terrain &ter = *terPtr;

	for (int workLOD = attainedLOD+1; workLOD <= desiredLOD; ++workLOD) {

		int numUnitsPerCell = 1 << workLOD;
		int heightMapIndexInc = (1 << desiredLOD) / numUnitsPerCell;

		// get delta t (0.0 <= t <= 1.0) for surface sampling
		float deltaT = 1.0f / (float)numUnitsPerCell;

		// find height points

		int iz = 0;
		for (int cv = 0; cv < chunkSize; ++cv) {	// for each patch within the chunk
		
			float t = 0;
			int endZ = (cv < chunkSize-1) ? numUnitsPerCell : numUnitsPerCell+1; // this is so the last row of vertices will be included
			for (int uz = 0; uz < endZ; ++uz) {	// for each vertex within a patch
			
				int ix = 0;	
				for (int ch = 0; ch < chunkSize; ++ch) {

					CubicBSpline::setMiddleMatrixPtr(chunkMatrices[cv * chunkSize + ch]);				
							
					float s = 0;
					int endX = (ch < chunkSize-1) ? numUnitsPerCell : numUnitsPerCell+1;
					for (int ux = 0; ux < endX; ++ux) {
						int heightMapIndex = iz*cacheNumVerts + ix;

						// if this point has not already been cached by a lower LOD in previous pass
						if (cacheSkipBuffer[heightMapIndex] != 1) {
							chunkHeights[heightMapIndex] = CubicBSpline::calcHeightOnPatchMatrix(s, t, true);
							chunkNormals[heightMapIndex] = CubicBSpline::calcNormalOnPatchMatrix(s, t, true);
							cacheSkipBuffer[heightMapIndex] = 1;
						}
				
						ix += heightMapIndexInc;
						s += deltaT;
					}
				}

				iz += heightMapIndexInc;
				t += deltaT;				
			}
		}

		// this LOD has been attained, check time budget to see if should continue with the next LOD this frame
		attainedLOD = workLOD;			
		if (timer.calcNumSecondsSinceTime(ter.cacheTimeStarted) >= ter.cacheMaxTimeInSec) {
			withinTimeBudget = false;
			break;
		}
	}

	// if full LOD has been cached free up memory taken by cacheSkipBuffer
	if (desiredLOD == attainedLOD) {		
		delete [] cacheSkipBuffer;
		cacheSkipBuffer = 0;
	}

	return withinTimeBudget;
}


TerrainChunkCache::TerrainChunkCache(const Terrain *ter, int chunkX, int chunkZ) : terPtr(ter)
{
	const Terrain &terRef = *ter;

	// set up matrices
	float hpBuffer[16];

	chunkSize = terRef.chunkSize;
	cellX = chunkX * chunkSize;
	cellZ = chunkZ * chunkSize;

	chunkMatrices = new Matrix4x4[chunkSize * chunkSize];
	for (int mz = 0; mz < chunkSize; ++mz) {
		for (int mx = 0; mx < chunkSize; ++mx) {
			// get 16 control points used for spline patch
			terRef.getBSplineControlPointsFromHeightmap(cellX+mx, cellZ+mz, hpBuffer);
			CubicBSpline::preCalcMiddleMatrix(hpBuffer); // pre calculate the middle matrices to find heights and normals
			chunkMatrices[mz*chunkSize+mx].set(CubicBSpline::getMiddleMatrix());
		}
	}

	// get number of units for this patch
	desiredLOD = terRef.tLODHandler->getMaxChunkLOD(chunkX, chunkZ);	

	int numUnitsPerCell = 1 << desiredLOD;

	// allocate height and normal buffer memory
	cacheNumVerts = chunkSize * numUnitsPerCell + 1; // add one for last row and column of vertices
	chunkHeights = new float[cacheNumVerts * cacheNumVerts];
	chunkNormals = new Vector3[cacheNumVerts * cacheNumVerts];
	cacheSkipBuffer = new unsigned char[cacheNumVerts * cacheNumVerts];
	memset(cacheSkipBuffer,0,cacheNumVerts * cacheNumVerts);

	// run the interpolation caching step, may not attain full LOD first pass but will attain at least LOD level 0
	attainedLOD = -1;
	cacheHeightsAndNormals();
}

TerrainChunkCache::~TerrainChunkCache()
{
	delete [] chunkMatrices;
	delete [] chunkHeights;
	delete [] chunkNormals;
	if (desiredLOD != attainedLOD) delete [] cacheSkipBuffer;	
}
