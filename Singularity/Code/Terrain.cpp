/* ----==== TERRAIN.CPP ====---- */

#include "terrain.h"
#include "terrainoccluder.h"
#include "terrainlodhandler.h"
#include "terrainchunkcache.h"
#include "terrainshadow.h"
#include "detailmanager.h"
#include "console.h"
#include "texturemanager.h"
#include "shadermanager.h"
#include "UTILITYCODE\lookupmanager.h"
#include "MATHCODE\spline.h"

#include "engine.h" ///// TEMP for wireframe drawing
#include "world.h" ///// TEMP for sky info, print sunAngle

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Terrain //////////

float Terrain::getHeightBSpline(float _x, float _z/*, Vector3 *normal, float *concavity*/) const
{
	const float invSpacing = 1.0f / hSpacing;
	const float fx = _x * invSpacing;
	const float fz = _z * invSpacing;
	const int ix = (int)fx;
	const int iz = (int)fz;

	///// if map boundaries are violated
	if (ix < 0 || ix > size-2 || iz < 0 || iz > size-2) return 0;

	float dx = fx - ix;
	float dz = fz - iz;

	int cellX = (int)(_x / hSpacing);
	int cellZ = (int)(_z / hSpacing);

	// cell player is on is already cached
	float hpBuffer[16];
	getBSplineControlPointsFromHeightmap(cellX, cellZ, hpBuffer);
	CubicBSpline::preCalcMiddleMatrix(hpBuffer); 

//	(*normal) = CubicBSpline::calcNormalOnPatchMatrix(dx, dz, false);
	//(*concavity) = tLODHandler->getChunkConcavity(cellX/chunkSize, cellZ/chunkSize);
//	(*concavity) = CubicBSpline::calcConcavityOnPatchMatrix(dx, dz, false); 
	return CubicBSpline::calcHeightOnPatchMatrix(dx, dz, false);
}


Vector3 Terrain::getNormalBSpline(float _x, float _z) const
{
	const float invSpacing = 1.0f / hSpacing;
	const float fx = _x * invSpacing;
	const float fz = _z * invSpacing;
	const int ix = (int)fx;
	const int iz = (int)fz;

	///// if map boundaries are violated
	if (ix < 0 || ix > size-2 || iz < 0 || iz > size-2) return Vector3(0,0,0);

	float dx = fx - ix;
	float dz = fz - iz;

	int cellX = (int)(_x / hSpacing);
	int cellZ = (int)(_z / hSpacing);

	// cell player is on is already cached
	float hpBuffer[16];
	getBSplineControlPointsFromHeightmap(cellX, cellZ, hpBuffer);
	CubicBSpline::preCalcMiddleMatrix(hpBuffer); 

	return CubicBSpline::calcNormalOnPatchMatrix(dx, dz, false);
}


const Matrix4x4 &Terrain::getCachedMatrixForCell(int cellX, int cellZ) const
{
	if (chunkCache[(cellZ/chunkSize)*numChunks+(cellX/chunkSize)] == 0) {
		console.addLineErr("Matrix is not cached for requested cell %i,%i", cellX,cellZ);
		chunkCache[(cellZ/chunkSize)*numChunks+(cellX/chunkSize)] = new TerrainChunkCache(this, cellX/chunkSize, cellZ/chunkSize);
		console.addLineErr("Cache miss: created matrix for chunk %i,%i", cellX/chunkSize,cellZ/chunkSize);
	}
	msgAssert(chunkCache[(cellZ/chunkSize)*numChunks+(cellX/chunkSize)] != 0, "Matrix is not cached for requested cell");
	return chunkCache[(cellZ/chunkSize)*numChunks+(cellX/chunkSize)]->getMatrix(cellX, cellZ);
}


void Terrain::cacheChunks(const Camera &cam)
{
	///// New Chunks to cache
	cacheTimeStarted = timer.queryCurrentTime();

	float invChunkLength = 1.0f / chunkLength;
	// get distance from frustum origin to frustum corner
	float radius = cam.getFrustum().getDistToCorner() + 1.0f; // add 1.0 for precision errors

	// make a box around the player so all chunks that can be seen by rotating camera are already cached
	int minChunkX = static_cast<int>((cam.getP().x - radius) * invChunkLength); minChunkX = minChunkX < 0 ? 0 : minChunkX;
	int maxChunkX = static_cast<int>((cam.getP().x + radius) * invChunkLength); maxChunkX = maxChunkX >= numChunks ? numChunks-1 : maxChunkX;
	int minChunkZ = static_cast<int>((cam.getP().z - radius) * invChunkLength); minChunkZ = minChunkZ < 0 ? 0 : minChunkZ;
	int maxChunkZ = static_cast<int>((cam.getP().z + radius) * invChunkLength); maxChunkZ = maxChunkZ >= numChunks ? numChunks-1 : maxChunkZ;

	// delete chunks that have moved out of cache box
	for (int z = cacheBoxMinChunkZ; z <= cacheBoxMaxChunkZ; ++z) {
		for (int x = cacheBoxMinChunkX; x <= cacheBoxMaxChunkX; ++x) {
			if (x < minChunkX || x > maxChunkX || z < minChunkZ || z > maxChunkZ) {
				delete chunkCache[z*numChunks+x];	// delete the cache structure (matrices)
				chunkCache[z*numChunks+x] = 0;		// set the pointer to null				
			}
		}
	}
	// allocate new chunk cache structures
	for (int z2 = minChunkZ; z2 <= maxChunkZ; ++z2) {
		for (int x = minChunkX; x <= maxChunkX; ++x) {
			if (x < cacheBoxMinChunkX || x > cacheBoxMaxChunkX || z2 < cacheBoxMinChunkZ || z2 > cacheBoxMaxChunkZ) {
				msgAssert(chunkCache[z2*numChunks+x] == 0, "Chunk pointer should be null");
				chunkCache[z2*numChunks+x] = new TerrainChunkCache(this, x, z2);
			
			// this is if it should be cached but isn't (first frame)
			} else if (chunkCache[z2*numChunks+x] == 0) {
				chunkCache[z2*numChunks+x] = new TerrainChunkCache(this, x, z2);
			}
		}
	}

	// refresh storage variables for this frame
	cacheBoxMinChunkX = minChunkX;
	cacheBoxMinChunkZ = minChunkZ;
	cacheBoxMaxChunkX = maxChunkX;
	cacheBoxMaxChunkZ = maxChunkZ;

	///// Continue with caching chunks where attainedLOD < desiredLOD

	// start caching nearest the camera and move outward

//	int chunksCached = 0; // TEMP
//	int levelsCached = 0; // TEMP

	bool continueCaching = true;
	while (continueCaching) {
		bool needsCaching = false;
		for (int z3 = minChunkZ; z3 <= maxChunkZ; ++z3) {
			for (int x = minChunkX; x <= maxChunkX; ++x) {
				needsCaching = chunkCache[z3*numChunks+x]->needsCaching();
				if (needsCaching) {
					int beforeLOD = chunkCache[z3*numChunks+x]->getAttainedLOD();
					continueCaching = chunkCache[z3*numChunks+x]->cacheHeightsAndNormals();
//					++chunksCached;
//					levelsCached += chunkCache[z*numChunks+x]->getAttainedLOD()-beforeLOD;
					if (!continueCaching) {
						/*if (chunksCached > 0) {
							float timeSpent = timer.calcNumSecondsSinceTime(cacheTimeStarted);
							console.addLine("%i chunks cached:   %i levels cached:   in %0.5fs", chunksCached, levelsCached, timeSpent);
						}*/
						break;
					} 
				}				
			}
			if (!continueCaching) break;
		}
		if (!needsCaching) break;
	}	
}


void Terrain::forceReCache(void)
{
	for (int c = 0; c < numChunks*numChunks; ++c) {
		delete chunkCache[c];
		chunkCache[c] = 0;
	}
}


/*----------------------------------------------------------------------------------------------------
void Terrain::checkBlendDistance
	checks to see if the chunk needs to be rendered with alpha blending because it is distant, also
	will check for near chunks needing detail layers and return value in bool &needsDetail
----------------------------------------------------------------------------------------------------*/
bool Terrain::checkBlendDistance(int cellX, int cellZ, float posX, float posZ,
								 float fadeStartDistSq, const Camera &cam) const
{
	// get dist squared to four corners to see if it needs alpha blanding for distant terrain
	CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(cellX, cellZ));
	Vector3 corner(posX, CubicBSpline::calcHeightOnPatchMatrix( 0, 0, true), posZ);		// upper left
	float dist = cam.getP().distSquared(corner);
	if (dist >= fadeStartDistSq) return true;

	posX += chunkLength;
	cellX += chunkSize-1;
	CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(cellX, cellZ));
	corner.assign(posX, CubicBSpline::calcHeightOnPatchMatrix(1.0f, 0, true), posZ);	// upper right
	dist = cam.getP().distSquared(corner);
	if (dist >= fadeStartDistSq) return true;

	posZ += chunkLength;
	cellZ += chunkSize-1;
	CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(cellX, cellZ));
	corner.assign(posX, CubicBSpline::calcHeightOnPatchMatrix(1.0f, 1.0f, true), posZ);	// lower right
	dist = cam.getP().distSquared(corner);
	if (dist >= fadeStartDistSq) return true;

	posX -= chunkLength;
	cellX -= chunkSize-1;
	CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(cellX, cellZ));
	corner.assign(posX, CubicBSpline::calcHeightOnPatchMatrix( 0, 1.0f, true), posZ);	// lower left
	dist = cam.getP().distSquared(corner);
	if (dist >= fadeStartDistSq) return true;

	return false;
}


void Terrain::drawTerrain(const Camera &cam)
{
	// bind the terrain shader
	GLhandleARB sID = shader.getShaderID(terShaderID);
	glUseProgramObjectARB(sID);

	// set vertex shader uniforms
	glUniform1fARB(glGetUniformLocationARB(sID,"terrainMaxHeight"), maxTerrainHeight);
	glUniform3fARB(glGetUniformLocationARB(sID,"camPosition"), cam.getP().x, cam.getP().y, cam.getP().z);

	// set fragment shader uniforms
	glUniform1fARB(glGetUniformLocationARB(sID,"startFade"), cam.getViewDist()*0.8f);
	glUniform1fARB(glGetUniformLocationARB(sID,"endFade"), cam.getViewDist());
	glUniform1fARB(glGetUniformLocationARB(sID,"detailStart"), engine.drawDetailLayer ? detailDist*0.25f : 0.0f);
	glUniform1fARB(glGetUniformLocationARB(sID,"detailEnd"), engine.drawDetailLayer ? detailDist : 0.0f);
	glUniform1fARB(glGetUniformLocationARB(sID,"sunAngle"), level.getSkyBox().getSunAngle());

	glUniform1iARB(glGetUniformLocationARB(sID,"TILE_1"), 0);
	glUniform1iARB(glGetUniformLocationARB(sID,"TILE_2"), 1);
	glUniform1iARB(glGetUniformLocationARB(sID,"TILE_3"), 2);
	glUniform1iARB(glGetUniformLocationARB(sID,"TILE_4"), 3);
	glUniform1iARB(glGetUniformLocationARB(sID,"LOOKUP"), 4);
	glUniform1iARB(glGetUniformLocationARB(sID,"LAYER_1"), 5);
	glUniform1iARB(glGetUniformLocationARB(sID,"LAYER_2"), 6);
	glUniform1iARB(glGetUniformLocationARB(sID,"LAYER_3"), 7);
	glUniform1iARB(glGetUniformLocationARB(sID,"LAYER_4"), 8);
	glUniform1iARB(glGetUniformLocationARB(sID,"LAYER_MAP"), 9);
	glUniform1iARB(glGetUniformLocationARB(sID,"NORMALMAP"), 10);
	glUniform1iARB(glGetUniformLocationARB(sID,"HORIZONMAP"), 11);

	// bind the textures
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(groundTexID[0]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(groundTexID[1]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE2_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(groundTexID[2]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE3_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(groundTexID[3]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE4_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(lookupTexID[0]).getGLTexID());


	glActiveTextureARB(GL_TEXTURE5_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(layerTexID[0]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE6_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(layerTexID[1]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE7_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(layerTexID[2]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE8_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(layerTexID[3]).getGLTexID());

	glActiveTextureARB(GL_TEXTURE9_ARB);
	glBindTexture(GL_TEXTURE_2D, texture.getTexture(layerMapTexID[0]).getGLTexID());

	if (tShadow->isNormalMapLoaded()) {
		glActiveTextureARB(GL_TEXTURE10_ARB);
		glBindTexture(GL_TEXTURE_2D, texture.getTexture(tShadow->getNormalMapTexID()).getGLTexID());
	}

	if (tShadow->isHorizonMapLoaded()) {
		glActiveTextureARB(GL_TEXTURE11_ARB);
		glBindTexture(GL_TEXTURE_2D, texture.getTexture(tShadow->getHorizonMapTexID()).getGLTexID());
	}

	// find the visible chunks
	tOccluder->scanConvertFrustum(	cam.getFrustum().getWorldPoint(0), cam.getFrustum().getWorldPoint(1),
									cam.getFrustum().getWorldPoint(2), cam.getFrustum().getWorldPoint(3),
									cam.getFrustum().getWorldPoint(4), cam.getFrustum().getLookDir());
	
	// set spacing values for normal calculation
	CubicBSpline::setSpacing(hSpacing, vSpacing);
	// calc LOD levels of cached chunks
	tLODHandler->clearFrameLODLevelMap(); 
	tLODHandler->computeFrameLODLevels(cam.getP());
	
	// cache the terrain chunk matrices	
	cacheChunks(cam);

	if (engine.wireframe) glPolygonMode(GL_FRONT,GL_LINE);	// WIREFRAME

	// draw the terrain chunks
	for (int c = 0; c < tOccluder->getVisibleList().size(); ++c) {
		// occluder returns an index, so we must fetch the cells like this
		int cellZ = (tOccluder->getVisibleList()[c] / numChunks) * chunkSize;
		int cellX = (tOccluder->getVisibleList()[c] % numChunks) * chunkSize;
		
		if (engine.drawCached) {
			drawCachedChunk(cellX, cellZ, cam);
		} else {
			drawChunk(cellX, cellZ, cam);
		}
	}

	if (engine.wireframe) glPolygonMode(GL_FRONT, GL_FILL);	// WIREFRAME

	// clean up
	glUseProgramObjectARB(0);
	glActiveTextureARB(GL_TEXTURE0_ARB);
}


// check to see that this can be ommitted if entire chunk fits within detail chunk range
// returns if this chunk needs a detail layer
void Terrain::drawChunk(int cellX, int cellZ, const Camera &cam) const
{
	float posX = (float)cellX * hSpacing;
	float posZ = (float)cellZ * hSpacing;

	// check here if this chunk needs alpha blending in the distance
	float fadeStartDistSq = cam.getViewDist() * 0.8f;	// use 80% of frustum view distance to start fade
	fadeStartDistSq *= fadeStartDistSq;

	bool needsAlpha = checkBlendDistance(cellX, cellZ, posX, posZ, fadeStartDistSq, cam);

	if (needsAlpha) {
		glEnable(GL_BLEND);
	} else {
		glColor4f(1,1,1,1);
	}

	// draw the chunk
	int chunkX = cellX / chunkSize;	// integer division
	int chunkZ = cellZ / chunkSize;
	int lod = tLODHandler->getChunkLOD(chunkX, chunkZ);	// get lods (-1 for boundaries)
	int lodMinusX = (chunkX-1 >= 0) ? tLODHandler->getChunkLOD(chunkX-1, chunkZ) : -1;
	int lodMinusZ = (chunkZ-1 >= 0) ? tLODHandler->getChunkLOD(chunkX, chunkZ-1) : -1;
	int lodPlusX = (chunkX+1 < numChunks) ? tLODHandler->getChunkLOD(chunkX+1, chunkZ) : -1;
	int lodPlusZ = (chunkZ+1 < numChunks) ? tLODHandler->getChunkLOD(chunkX, chunkZ+1) : -1;

	if (lod > -1) {
		drawBSplineChunk(cellX, cellZ, lod, lodMinusX, lodMinusZ, lodPlusX, lodPlusZ);
	} else {
		drawBSplineChunkOptimized(cellX, cellZ, lodMinusX, lodMinusZ, lodPlusX, lodPlusZ);
	}

	if (needsAlpha) glDisable(GL_BLEND);
}


void Terrain::drawCachedChunk(int cellX, int cellZ, const Camera &cam) const
{
	const TerrainLODHandler &tLOD = *tLODHandler;

	float posX = (float)cellX * hSpacing;
	float posZ = (float)cellZ * hSpacing;

	// check here if this chunk needs alpha blending in the distance
	float fadeStartDistSq = cam.getViewDist() * 0.8f;	// use 80% of frustum view distance to start fade
	fadeStartDistSq *= fadeStartDistSq;

	bool needsAlpha = checkBlendDistance(cellX, cellZ, posX, posZ, fadeStartDistSq, cam);

	if (needsAlpha) {
		glEnable(GL_BLEND);
	} else {
		glColor4f(1,1,1,1);
	}
	
	int chunkX = cellX / chunkSize;	// integer division
	int chunkZ = cellZ / chunkSize;

	// get LOD of this chunk
	int lod = tLOD.getCachedChunkLOD(chunkX, chunkZ);
	int maxLOD = tLOD.getMaxChunkLOD(chunkX, chunkZ);

	// get LODs of bordering chunks: MinusX, MinusZ, PlusX, PlusZ
	int edgeLODs[4], edgeMaxLODs[4];
	if (chunkX-1 >= cacheBoxMinChunkX) {
		edgeLODs[0] = tLOD.getCachedChunkLOD(chunkX-1, chunkZ);
		edgeMaxLODs[0] = tLOD.getMaxChunkLOD(chunkX-1, chunkZ);
	} else {
		edgeLODs[0] = lod;
		edgeMaxLODs[0] = maxLOD;
	}
	if (chunkZ-1 >= cacheBoxMinChunkZ) {
		edgeLODs[1] = tLOD.getCachedChunkLOD(chunkX, chunkZ-1);
		edgeMaxLODs[1] = tLOD.getMaxChunkLOD(chunkX, chunkZ-1);
	} else {
		edgeLODs[1] = lod;
		edgeMaxLODs[1] = maxLOD;
	}
	if (chunkX+1 <= cacheBoxMaxChunkX) {
		edgeLODs[2] = tLOD.getCachedChunkLOD(chunkX+1, chunkZ);
		edgeMaxLODs[2] = tLOD.getMaxChunkLOD(chunkX+1, chunkZ);
	} else {
		edgeLODs[2] = lod;
		edgeMaxLODs[2] = maxLOD;
	}
	if (chunkZ+1 <= cacheBoxMaxChunkZ) {
		edgeLODs[3] = tLOD.getCachedChunkLOD(chunkX, chunkZ+1);
		edgeMaxLODs[3] = tLOD.getMaxChunkLOD(chunkX, chunkZ+1);
	} else {
		edgeLODs[3] = lod;
		edgeMaxLODs[3] = maxLOD;
	}

	// draw the chunk
	drawCachedBSplineChunk(cellX, cellZ, lod, maxLOD, edgeLODs, edgeMaxLODs);

	if (needsAlpha) glDisable(GL_BLEND);
}


/*----------------------------------------------------------------------------------------------------
void Terrain::getBSplineControlPointsFromHeightmap
	modifies float *buffer with worldspace heights
----------------------------------------------------------------------------------------------------*/
void Terrain::getBSplineControlPointsFromHeightmap(int cellX, int cellZ, float *buffer) const
{
	int heightZ = cellZ - 1;
	for (int z = 0; z < 4; ++z) {
		int heightX = cellX - 1;
		for (int x = 0; x < 4; ++x) {
			if (heightX < 0 || heightZ < 0 || heightX >= size || heightZ >= size) { // beyond edge of heightmap, make 0 height
				buffer[z*4+x] = 0;
			} else {
				buffer[z*4+x] = vSpacing * (float)heightMap[heightZ*size+heightX];
			}
			++heightX;
		}
		++heightZ;
	}
}


/*----------------------------------------------------------------------------------------------------
void Terrain::drawBSplineChunk
Params: lod of chunk and the adjacent chunks. Edges may only scale down, not up, so edges will either
	be same or lower LOD. Values (n) go from 0 up, resulting (2^n)+1 vertices along each axis of the
	patch, 2^n units. Value of -1 for edge means neighbor is optimized chunk, so use this lod for edge
        -z	AXES for this function
	     |
	-x --0-- +x
	     |
	    +z
----------------------------------------------------------------------------------------------------*/
void Terrain::drawBSplineChunk(int cellX, int cellZ, int lod, int lodMinusX, int lodMinusZ, int lodPlusX, int lodPlusZ) const
{
	// fix lods
	if (lodPlusX  > lod || lodPlusX  == -1) lodPlusX  = lod;
	if (lodMinusX > lod || lodMinusX == -1) lodMinusX = lod;
	if (lodPlusZ  > lod || lodPlusZ  == -1) lodPlusZ  = lod;
	if (lodMinusZ > lod || lodMinusZ == -1) lodMinusZ = lod;
	// get values used to draw complete corners or not with edges
	unsigned char mZ = (lodMinusZ == lod) ? 1 : 0;
	unsigned char pZ = (lodPlusZ  == lod) ? 1 : 0;
	unsigned char mX = (lodMinusX == lod) ? 1 : 0;
	unsigned char pX = (lodPlusX  == lod) ? 1 : 0;

	// get number of units for this patch and its edges
	int numUnits = 1 << lod;
	// get delta t (0.0 <= t <= 1.0) for surface sampling
	float deltaT = 1.0f / (float)numUnits;
	// get world space distances between sample points
	float spacingT = deltaT * hSpacing;

	// draw the chunk
	float startPosX = cellX * hSpacing;
	float startPosZ = cellZ * hSpacing;

	// draw top left edges
	if (lodMinusX < lod) drawBSplineChunkEdge(cellX, cellZ, lod, lodMinusX, deltaT, spacingT, 0, pZ, mZ); // -x
	if (lodMinusZ < lod) drawBSplineChunkEdge(cellX, cellZ, lod, lodMinusZ, deltaT, spacingT, 1, mX, pX); // -z

	// draw center section
	glBegin(GL_TRIANGLE_STRIP);
		float posZ = startPosZ;
		for (int v = 0; v < chunkNumVerts-1; ++v) {	// for each patch within the chunk

			// find whether or not to draw Z edges
			int uzStart, uzEnd;
			float uzPos, t;
			if (lodMinusZ < lod && v == 0) {
				uzStart = 1;
				uzEnd = (lodPlusZ < lod && v == chunkNumVerts-2) ? numUnits-2 : numUnits-1;
				uzPos = posZ + spacingT;
				t = deltaT;
			} else {
				uzStart = 0;				
				uzEnd = (lodPlusZ < lod && v == chunkNumVerts-2) ? numUnits-2 : numUnits-1;
				uzPos = posZ;
				t = 0;
			}
						
			for (int uz = uzStart; uz <= uzEnd; ++uz) {	// for each vertex within a patch
				
				float posX = startPosX;
				for (int h = 0; h < chunkNumVerts-1; ++h) {
			
					CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(cellX+h, cellZ+v));
			
					int uxStart, uxEnd;
					float uxPos, s, texU;
					if (lodMinusX < lod && h == 0) {
						uxStart = 1;
						uxEnd = (lodPlusX < lod && h == chunkNumVerts-2) ? numUnits-2: numUnits-1;
						uxPos = posX + spacingT;
						s = deltaT;
					} else {
						uxStart = 0;
						uxEnd = (lodPlusX < lod && h == chunkNumVerts-2) ? numUnits-2: numUnits-1;
						uxPos = posX;
						s = 0;
					}									// last two vertices of each patch are not drawn because they are the same
					if (h == chunkNumVerts-2) ++uxEnd;	// as the first two of the next patch, but on the last patch of each row
														// they must be drawn to end the row properly
					
					for (int ux = uxStart; ux <= uxEnd; ++ux) {
						Vector3 p(	uxPos,
									CubicBSpline::calcHeightOnPatchMatrix(s, t, true),
									uzPos);
						// texture coordinate for terrain textures
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						// texture coordinate for the horizon map
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						// texture coordinate for the detail texture
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(s, t, true).v);
						glVertex3fv(p.v);
						// if not first row do this twice for degenerate tri for strip						
						if ((v > 0 || uz > uzStart) && h == 0 && ux == uxStart) glVertex3fv(p.v);
			
						p.assign(	uxPos,
									CubicBSpline::calcHeightOnPatchMatrix(s, t+deltaT, true),
									uzPos+spacingT);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(s, t+deltaT, true).v);
						glVertex3fv(p.v);
						// extra for degenerate tri for strip - ux will only be numUnits on last vertex of last patch in row
						// (or numUnits-1 in the case of lodPlusX being less than lod)
						if (h == chunkNumVerts-2) {
							if ((lodPlusX < lod && ux == numUnits-1) || (ux == numUnits)) {
								glVertex3fv(p.v);
							}
						}
	
						uxPos += spacingT;
						s += deltaT;
					}

					posX += hSpacing;
				}

				uzPos += spacingT;
				t += deltaT;				
			}
			
			posZ += hSpacing;
		}
	glEnd();

	// draw bottom right edges
	if (lodPlusX < lod) drawBSplineChunkEdge(cellX, cellZ, lod, lodPlusX, deltaT, spacingT, 2, mZ, pZ); // +x
	if (lodPlusZ < lod) drawBSplineChunkEdge(cellX, cellZ, lod, lodPlusZ, deltaT, spacingT, 3, pX, mX); // +z
}


// edgeLODs in this order: MinusX, MinusZ, PlusX, PlusZ
void Terrain::drawCachedBSplineChunk(int cellX, int cellZ, int lod, int maxLOD, int *edgeLODs, int *edgeMaxLODs) const
{
	TerrainChunkCache &cacheRef = *chunkCache[(cellZ/chunkSize)*numChunks+(cellX/chunkSize)];	

	int lodDiff = maxLOD - lod;
	int vertInc = 1 << lodDiff;

	// get number of units for this patch and its edges
	int numUnits = 1 << lod;
	int numVerts = chunkSize * numUnits + 1;
	// get world space distances between sample points
	float spacingT = hSpacing * chunkSize / ((float)(numVerts-1));

	int edgeIncMinusX = numUnits / (1 << edgeLODs[0]); edgeIncMinusX = (edgeIncMinusX < 1 ? 1 : edgeIncMinusX);
	int edgeIncMinusZ = numUnits / (1 << edgeLODs[1]); edgeIncMinusZ = (edgeIncMinusZ < 1 ? 1 : edgeIncMinusZ);
	int edgeIncPlusX = numUnits / (1 << edgeLODs[2]); edgeIncPlusX = (edgeIncPlusX < 1 ? 1 : edgeIncPlusX);
	int edgeIncPlusZ = numUnits / (1 << edgeLODs[3]); edgeIncPlusZ = (edgeIncPlusZ < 1 ? 1 : edgeIncPlusZ);

	// draw the chunk
	float startPosX = cellX * hSpacing;
	float startPosZ = cellZ * hSpacing;

	// draw center section
	glBegin(GL_TRIANGLE_STRIP);
		
		int iz = 0;
		float posZ = startPosZ;
		for (int pz = 0; pz < numVerts-1; ++pz) {	// for each vertex in the row of the chunk
			
			int ix = 0;
			float posX = startPosX;
			for (int px = 0; px < numVerts; ++px) {
				Vector3 p;
				// get terrain vertex
				if (px == 0 && lod > edgeLODs[0] && pz % edgeIncMinusX > 0) { // left edge with different LOD
					int edgeLowIZ = (pz / edgeIncMinusX) * (vertInc * edgeIncMinusX);
					float hDiffInc = (cacheRef.getHeight(ix, edgeLowIZ+(vertInc*edgeIncMinusX)) -
						cacheRef.getHeight(ix, edgeLowIZ)) / (float)edgeIncMinusX;
					p.assign(posX, cacheRef.getHeight(ix, edgeLowIZ) + (hDiffInc * (pz % edgeIncMinusX)), posZ);					
				} else if (px == numVerts-1 && lod > edgeLODs[2] && pz % edgeIncPlusX > 0) { // right edge with different LOD
					int edgeLowIZ = (pz / edgeIncPlusX) * (vertInc * edgeIncPlusX);
					float hDiffInc = (cacheRef.getHeight(ix, edgeLowIZ+(vertInc*edgeIncPlusX)) -
						cacheRef.getHeight(ix, edgeLowIZ)) / (float)edgeIncPlusX;
					p.assign(posX, cacheRef.getHeight(ix, edgeLowIZ) + (hDiffInc * (pz % edgeIncPlusX)), posZ);
				} else if (pz == 0 && lod > edgeLODs[1] && px % edgeIncMinusZ > 0) { // upper edge with different LOD
					int edgeLowIX = (px / edgeIncMinusZ) * (vertInc * edgeIncMinusZ);
					float hDiffInc = (cacheRef.getHeight(edgeLowIX+(vertInc*edgeIncMinusZ), iz) -
						cacheRef.getHeight(edgeLowIX, iz)) / (float)edgeIncMinusZ;
					p.assign(posX, cacheRef.getHeight(edgeLowIX, iz) + (hDiffInc * (px % edgeIncMinusZ)), posZ);
				} else { // rest of chunk
					p.assign(posX, cacheRef.getHeight(ix, iz), posZ);
				}

				// texture coordinate for the terrain textures
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
				// texture coordinate for the horizon map
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
				// texture coordinate for the detail texture
				glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
				glNormal3fv(cacheRef.getNormal(ix, iz).v);				
				glVertex3fv(p.v);
				
				// if not first row do this twice for degenerate tri for strip						
				if (pz > 0 && px == 0) glVertex3fv(p.v);

				int pz2 = pz + 1;
				if (px == 0 && lod > edgeLODs[0] && pz2 % edgeIncMinusX > 0) { // left edge with different LOD
					int edgeLowIZ = (pz2 / edgeIncMinusX) * (vertInc * edgeIncMinusX);
					float hDiffInc = (cacheRef.getHeight(ix, edgeLowIZ+(vertInc*edgeIncMinusX)) -
						cacheRef.getHeight(ix, edgeLowIZ)) / (float)edgeIncMinusX;
					p.assign(posX, cacheRef.getHeight(ix, edgeLowIZ) + (hDiffInc * (pz2 % edgeIncMinusX)), posZ+spacingT);
				} else if (px == numVerts-1 && lod > edgeLODs[2] && pz2 % edgeIncPlusX > 0) { // right edge with different LOD
					int edgeLowIZ = (pz2 / edgeIncPlusX) * (vertInc * edgeIncPlusX);
					float hDiffInc = (cacheRef.getHeight(ix, edgeLowIZ+(vertInc*edgeIncPlusX)) -
						cacheRef.getHeight(ix, edgeLowIZ)) / (float)edgeIncPlusX;
					p.assign(posX, cacheRef.getHeight(ix, edgeLowIZ) + (hDiffInc * (pz2 % edgeIncPlusX)), posZ+spacingT);
				} else if (pz == numVerts-2 && lod > edgeLODs[3] && px % edgeIncPlusZ > 0) {
					int edgeLowIX = (px / edgeIncPlusZ) * (vertInc * edgeIncPlusZ);
					float hDiffInc = (cacheRef.getHeight(edgeLowIX+(vertInc*edgeIncPlusZ), iz+vertInc) -
						cacheRef.getHeight(edgeLowIX, iz+vertInc)) / (float)edgeIncPlusZ;
					p.assign(posX, cacheRef.getHeight(edgeLowIX, iz+vertInc) + (hDiffInc * (px % edgeIncPlusZ)), posZ+spacingT);
				} else {
					p.assign(posX, cacheRef.getHeight(ix, iz+vertInc), posZ+spacingT);
				}
				
				glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
				glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
				glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
				glNormal3fv(cacheRef.getNormal(ix, iz+vertInc).v);				
				glVertex3fv(p.v);

				// extra for degenerate tri for strip - ux will only be numUnits on last vertex of last patch in row
				if (px == numVerts-1 && pz != numVerts-2) glVertex3fv(p.v);				
	
				posX += spacingT;
				ix += vertInc;
			}
		
			posZ += spacingT;
			iz += vertInc;
		}
	glEnd();
}


/*----------------------------------------------------------------------------------------------------
void Terrain::drawBSplineChunkOptimized
Params: lod of chunk and the adjacent chunks. Optimized chunks are those with LOD -1 and are intended
	only for those chunks where all control points are at the same elevation, making a flat surface.
	Unlike other chunks where edges can only be equal or lower LODs, the optimized edges are equal or
	greater. Therefor optimized takes responsibility for preventing T-junctions by fanning to its
	neighboring LODs.
----------------------------------------------------------------------------------------------------*/
void Terrain::drawBSplineChunkOptimized(int cellX, int cellZ, int lodMinusX, int lodMinusZ, int lodPlusX, int lodPlusZ) const
{
	float startPosX = cellX * hSpacing;
	float startPosZ = cellZ * hSpacing;
	float height = (float)heightMap[(cellZ * size) + cellX] * vSpacing;
	Vector3 p;

	glBegin(GL_TRIANGLE_FAN);
		glNormal3f(0,1.0f,0);

		// from center
		p.assign(	startPosX + (chunkLength*0.5f),
					height,
					startPosZ + (chunkLength*0.5f));
		glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
		glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
		glVertex3fv(p.v);

		int numUnits;
		float spacing;

		// to upper right: top -z
		float currPos = startPosX + chunkLength;
		if (lodMinusZ == -1) {
			numUnits = 1;
			spacing = chunkLength;
		} else {
			numUnits = (1 << lodMinusZ) * chunkSize;
			spacing = chunkLength / (float)numUnits;
		}
		for (int f = 0; f < numUnits; ++f) {
			p.assign(	currPos,
						height,
						startPosZ);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
			glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
			glVertex3fv(p.v);

			currPos -= spacing;
		}
	
		// left -x
		currPos = startPosZ;
		if (lodMinusX == -1) {
			numUnits = 1;
			spacing = chunkLength;
		} else {
			numUnits = (1 << lodMinusX) * chunkSize;
			spacing = chunkLength / (float)numUnits;
		}		
		for (int f2 = 0; f2 < numUnits; ++f2) {
			p.assign(	startPosX,
						height,
						currPos);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
			glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
			glVertex3fv(p.v);

			currPos += spacing;
		}
		
		// bottom +z
		currPos = startPosX;
		if (lodPlusZ == -1) {
			numUnits = 1;
			spacing = chunkLength;
		} else {
			numUnits = (1 << lodPlusZ) * chunkSize;
			spacing = chunkLength / (float)numUnits;
		}
		for (int f3 = 0; f3 < numUnits; ++f3) {
			p.assign(	currPos,
						height,
						startPosZ + chunkLength);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
			glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
			glVertex3fv(p.v);

			currPos += spacing;
		}

		// right +x
		currPos = startPosZ + chunkLength;
		if (lodPlusX == -1) {
			numUnits = 1;
			spacing = chunkLength;
		} else {
			numUnits = (1 << lodPlusX) * chunkSize;
			spacing = chunkLength / (float)numUnits;
		}
		for (int f4 = 0; f4 < numUnits+1; ++f4) { // extra one here to bring it back to upper left corner
			p.assign(	startPosX + chunkLength,
						height,
						currPos);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
			glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
			glVertex3fv(p.v);

			currPos -= spacing;
		}

	glEnd();
}


/*----------------------------------------------------------------------------------------------------
void drawBSplineChunkMinusXEdge
	    1		EDGE parameter		same axes as above function
	  |----|
	0 |    | 2	drawNearCorner and drawFarCorner (0 or 1) for cases where an adjacent side of the
	  |----|		chunk to this edge is same lod and drawn as part of the triangle strip, then
	    3			corner needs to be drawn by this edge instead of leaving it out
----------------------------------------------------------------------------------------------------*/
void Terrain::drawBSplineChunkEdge(int cellX, int cellZ, int innerLod, int outerLod, float innerDeltaT, float innerSpacing,
									unsigned char edge, unsigned char drawNearCorner, unsigned char drawFarCorner) const
{
	// get setup values
	int numUnitsInner = 1 << innerLod;
	int numUnitsOuter = 1 << outerLod;
	int lodDiff = innerLod - outerLod; 
	int numInnerRepeats = (1 << lodDiff) + 1;	
	int useCellX, useCellZ;

	// get world space values
	float posX, posZ, outerLoc;	
	float outerDeltaT = 1.0f / (float)numUnitsOuter;
	float outerSpacing = outerDeltaT * hSpacing;	

	switch (edge) {
		case 0: // -x
			useCellX = cellX;
			useCellZ = cellZ + chunkSize - 1;
			posX = cellX * hSpacing;
			posZ = cellZ * hSpacing;
			outerLoc = posZ + chunkLength;
			break;
		case 1: // -z
			useCellX = cellX;
			useCellZ = cellZ;
			posX = cellX * hSpacing;
			posZ = cellZ * hSpacing;
			outerLoc = posX;
			break;
		case 2: // +x
			useCellX = cellX + chunkSize - 1;
			useCellZ = cellZ;
			posX = cellX * hSpacing + chunkLength;
			posZ = cellZ * hSpacing;
			outerLoc = posZ;
			break;
		case 3: // +z
			useCellX = cellX + chunkSize - 1;
			useCellZ = cellZ + chunkSize - 1;
			posX = cellX * hSpacing;
			posZ = cellZ * hSpacing + chunkLength;
			outerLoc = posX + chunkLength;
			break;
	}

	float startPosX = posX;
	float startPosZ = posZ;

	int innerPos = 0;
	for (int patch = 0; patch < chunkSize; ++patch) {
		
		CubicBSpline::setMiddleMatrixPtr(getCachedMatrixForCell(useCellX, useCellZ));

		float innerLoc = 0, innerT = 0;
		float outerT = 0;

		for (int outerCount = 0; outerCount < numUnitsOuter; ++outerCount) { // controls how many fans are rendered
			Vector3 p;
			glBegin(GL_TRIANGLE_FAN);
				
				switch (edge) {
					case 0: // -x
						p.assign(	posX,
									CubicBSpline::calcHeightOnPatchMatrix(0, 1.0f - outerT, true),
									outerLoc - (outerCount*outerSpacing));
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(0, 1.0f - outerT, true).v);
						glVertex3fv(p.v);
						break;
					case 1: // -z
						p.assign(	outerLoc + (outerCount*outerSpacing),
									CubicBSpline::calcHeightOnPatchMatrix(outerT, 0, true),
									posZ);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(outerT, 0, true).v);
						glVertex3fv(p.v);
						break;
					case 2: // +x
						p.assign(	posX,
									CubicBSpline::calcHeightOnPatchMatrix(1.0f, outerT, true),
									outerLoc + (outerCount*outerSpacing));
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f, outerT, true).v);
						glVertex3fv(p.v);
						break;
					case 3: // +z
						p.assign(	outerLoc - (outerCount*outerSpacing),
									CubicBSpline::calcHeightOnPatchMatrix(1.0f - outerT, 1.0f, true),
									posZ);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f - outerT, 1.0f, true).v);
						glVertex3fv(p.v);
						break;
				}

				for (int innerCount = 0; innerCount < numInnerRepeats; ++innerCount) { // controls how many tris each fan contains
					// special case: do not render first and final inner index vertex
					if ((patch != 0 || innerPos != 0 || drawNearCorner == 1) &&
						(patch != chunkSize-1 || innerPos != numUnitsInner*chunkSize || drawFarCorner == 1)) {
						switch (edge) {
							case 0: // -x
								p.assign(	posX + innerSpacing,
											CubicBSpline::calcHeightOnPatchMatrix(innerDeltaT, 1.0f - innerT, true),
											outerLoc - innerLoc);
								glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
								glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
								glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(innerDeltaT, 1.0f - innerT, true).v);
								glVertex3fv(p.v);
								break;
							case 1: // -z
								p.assign(	outerLoc + innerLoc,
											CubicBSpline::calcHeightOnPatchMatrix(innerT, innerDeltaT, true),
											posZ + innerSpacing);
								glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
								glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
								glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(innerT, innerDeltaT, true).v);
								glVertex3fv(p.v);
								break;
							case 2: // +x
								p.assign(	posX - innerSpacing,
											CubicBSpline::calcHeightOnPatchMatrix(1.0f - innerDeltaT, innerT, true),
											outerLoc + innerLoc);
								glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
								glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
								glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f - innerDeltaT, innerT, true).v);
								glVertex3fv(p.v);
								break;
							case 3: // +z
								p.assign(	outerLoc - innerLoc,
											CubicBSpline::calcHeightOnPatchMatrix(1.0f - innerT, 1.0f - innerDeltaT, true),
											posZ - innerSpacing);
								glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
								glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
								glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
								glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f - innerT, 1.0f - innerDeltaT, true).v);
								glVertex3fv(p.v);
								break;
						}
					}

					if (innerCount < numInnerRepeats-1) { // don't increment if last inner position so next fan will use it again
						++innerPos;
						innerLoc += innerSpacing;
						innerT += innerDeltaT;
					}
				}

				switch (edge) {
					case 0: // -x
						p.assign(	posX,
									CubicBSpline::calcHeightOnPatchMatrix(0, 1.0f - outerT - outerDeltaT, true),
									outerLoc - ((outerCount+1)*outerSpacing));
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(0, 1.0f - outerT - outerDeltaT, true).v);
						glVertex3fv(p.v);
						break;
					case 1: // -z
						p.assign(	outerLoc + ((outerCount+1)*outerSpacing),
									CubicBSpline::calcHeightOnPatchMatrix(outerT + outerDeltaT, 0, true),
									posZ);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(outerT + outerDeltaT, 0, true).v);
						glVertex3fv(p.v);
						break;
					case 2: // +x
						p.assign(	posX,
									CubicBSpline::calcHeightOnPatchMatrix(1.0f, outerT + outerDeltaT, true),
									outerLoc + ((outerCount+1)*outerSpacing));
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f, outerT + outerDeltaT, true).v);
						glVertex3fv(p.v);
						break;
					case 3: // +z
						p.assign(	outerLoc - ((outerCount+1)*outerSpacing),
									CubicBSpline::calcHeightOnPatchMatrix(1.0f - outerT - outerDeltaT, 1.0f, true),
									posZ);
						glMultiTexCoord2fARB(GL_TEXTURE0_ARB, (p.x-startPosX)*texMult, (p.z-startPosZ)*texMult);
						glMultiTexCoord2fARB(GL_TEXTURE1_ARB, p.x*invMapLength, p.z*invMapLength);
						glMultiTexCoord2fARB(GL_TEXTURE2_ARB, (p.x-startPosX)*texMultDet, (p.z-startPosZ)*texMultDet);
						glNormal3fv(CubicBSpline::calcNormalOnPatchMatrix(1.0f - outerT - outerDeltaT, 1.0f, true).v);
						glVertex3fv(p.v);
						break;
				}

			glEnd();

			outerT += outerDeltaT;
		}

		switch (edge) {
			case 0: // -x
				outerLoc -= hSpacing;
				--useCellZ;
				break;
			case 1: // -z
				outerLoc += hSpacing;
				++useCellX;
				break;
			case 2: // +x
				outerLoc += hSpacing;
				++useCellZ;
				break;
			case 3: // +z
				outerLoc -= hSpacing;
				--useCellX;
				break;
		}
	}
}


void Terrain::createHorizonMap(int texSize, const std::string &filename)
{
	tShadow->createHorizonMap(texSize, filename);
}


/*----------------------------------------------------------------------------------------------------
void Terrain::loadRAW
	RAW should have 4 bytes for a 32 bit integer value giving dimensions, followed by data (1 byte per
	pixel for now)
----------------------------------------------------------------------------------------------------*/
bool Terrain::loadRAW(const std::string &filename)
{
	// heightmap Loading
	FILE *inFile;
	if (fopen_s(&inFile,filename.c_str(), "rb") != 0) return false;

	fseek(inFile,0,SEEK_SET);
	fread(&size,4,1,inFile);
	numChunks = (size-1)/chunkSize;

	// allocate memory
	heightMap = new unsigned char[size*size];

	// load the heightmap data
	fread(heightMap,sizeof(unsigned char),size*size,inFile);
	fclose(inFile);
	console.addLine("     terrain heightmap loaded: \"%s\": %ix%i", filename.c_str(), size, size);

	// set variables
	mapLength = (size-1)*hSpacing;
	invMapLength = mapLength == 0.0f ? 0.0f : 1.0f / mapLength;

	// create the terrain occluder
	tOccluder = new TerrainOccluder(this);

	// set spacing so concavity map will be accurate
	CubicBSpline::setSpacing(hSpacing, vSpacing);
	// create the LOD handler
	tLODHandler = new TerrainLODHandler(this);

	// create the chunk cache space - not actually allocating any chunks, just pointers
	chunkCache = new TerrainChunkCache*[numChunks*numChunks];
	for (int c = 0; c < numChunks*numChunks; ++c) chunkCache[c] = 0;	// initialize to null

	// create terrain shadow map
	tShadow = new TerrainShadow(this);

	///// temp sun direction
	lightDir.assign(math.getCos(math.degToIndex(175)), math.getSin(math.degToIndex(175)), 0);

	// load shadowmap
	//tShadow->loadFromRAW("./data/level/map_01_165-170-175.smp");
	tShadow->loadHorizonMap("./data/level/grasslands.hrz");

	return true;
}


Terrain::Terrain(int _chunkSize, float _hSpacing, float _vSpacing,
				 float _texStretch, float _detTexStretch, float _detailDist)
{
	size = 0;
	numChunks = 0;
	chunkSize = _chunkSize;
	chunkNumVerts = chunkSize + 1;
	mapLength = 0;
	invMapLength = 0;
	hSpacing = _hSpacing;
	vSpacing = _vSpacing;
	chunkLength = chunkSize * hSpacing;
	texStretch = _texStretch;
	detTexStretch = _detTexStretch;
	msgAssert(hSpacing != 0.0f, "Terrain::Terrain: hSpacing divide by 0");
	texMult = texStretch / hSpacing;
	texMultDet = detTexStretch / hSpacing;
	detailDist = _detailDist;
	maxTerrainHeight = (float)(1 << (sizeof(*heightMap)*8)) * vSpacing;

	heightMap = 0;

	groundTexID = 0;
	lookupTexID = 0;
	layerTexID = 0;
	layerMapTexID = 0;

	terShaderID = 0;
	detShaderID = 0;

	chunkCache = 0;
	cacheBoxMinChunkX = 0;
	cacheBoxMinChunkZ = 0;
	cacheBoxMaxChunkX = 0;
	cacheBoxMaxChunkZ = 0;
	cacheMaxTimeInSec = 0.002f;
	cacheTimeStarted = 0;

	tLODHandler = 0;
	tOccluder = 0;
	tShadow = 0;
	dManager = 0;
}


Terrain::~Terrain()
{
	delete [] groundTexID;
	delete [] lookupTexID;
	delete [] layerTexID;
	delete [] layerMapTexID;
	delete [] heightMap;

	delete tLODHandler;
	delete tOccluder;
	delete tShadow;
	delete dManager;

	for (int c = 0; c < numChunks*numChunks; ++c) delete chunkCache[c];	// delete the cached chunkcs
	delete [] chunkCache;	// delete the cache space
}

////////// old functions
/*
float Terrain::getHeight(float _x, float _z) const
{
	const float invSpacing = 1.0f / hSpacing;
	const float fx = _x * invSpacing;
	const float fz = _z * invSpacing;
	const int ix = (int)fx;
	const int iz = (int)fz;

	///// if map boundaries are violated
	if (ix < 0 || ix > size-2 || iz < 0 || iz > size-2) return 0;

/*	The following conditional assumes each terrain quad has this geometry
	_______
	|    /|
	|  /  |
	|/    |
	-------
*//*
	float dx = fx - ix;
	float dz = fz - iz;

	int index = iz * size + ix;

	if (dx >= 1.0f-dz) {	// bottom right tri of quad
		float xdiff = (float)(heightMap[index+size+1] - heightMap[index+size]);
		float zdiff = (float)(heightMap[index+size+1] - heightMap[index+1]);
		
		return (heightMap[index+size+1] - (xdiff * (1.0f-dx)) - (zdiff * (1.0f-dz))) * vSpacing;
	
	} else {				// upper left tri of quad
		float xdiff = (float)(heightMap[index] - heightMap[index+1]);
		float zdiff = (float)(heightMap[index] - heightMap[index+size]);
		
		return (heightMap[index] - (xdiff * dx) - (zdiff * dz)) * vSpacing;
	}
}
*/
/*
void Terrain::drawChunkVBO(int cellX, int cellZ, const Camera &cam) const
{
	float posX = (float)cellX * hSpacing;
	float posZ = (float)cellZ * hSpacing;

	// check here if this chunk needs alpha blending in the distance
	bool needsAlpha = false;
	float fadeStartDistSq = cam.getViewDist() * 0.8f;	// use 80% of frustum view distance to start fade
	fadeStartDistSq *= fadeStartDistSq;
	float diff = cam.getViewDist()*cam.getViewDist() - fadeStartDistSq;

	needsAlpha = checkBlendDistance(cellX,cellZ,posX,posZ,fadeStartDistSq,cam);
	if (needsAlpha) {
		glEnable(GL_BLEND);
	} else {
		glColor4f(1,1,1,1);
	}

	// draw the chunk
	for (int l = 0; l < chunkNumVerts; l++) {
		glDrawArrays(GL_LINE_STRIP,(cellZ+l)*size+cellX,chunkNumVerts);
	}
//	glDrawElements(GL_TRIANGLE_STRIP, indexBufferSize, GL_UNSIGNED_SHORT, indexBuffer);
//	glDrawRangeElements(GL_TRIANGLE_STRIP, indexBuffer[0], indexBuffer[indexBufferSize-1],
//						indexBufferSize, GL_UNSIGNED_SHORT, indexBuffer);

	if (needsAlpha) glDisable(GL_BLEND);
}
*/
/*
void Terrain::buildNormalMap(void)
{
	// calculate smooth vertex normals
	for (int z = 1; z < size-1; z++) {
		for (int x = 1; x < size-1; x++) {

			// get adjacent terrain points
			Vector3 tVert(		(float)x*hSpacing,		(float)heightMap[z*size+x]*vSpacing,	(float)z*hSpacing);
			Vector3 tVertLeft(	(float)(x-1)*hSpacing,	(float)heightMap[z*size+(x-1)]*vSpacing,(float)z*hSpacing);
			Vector3 tVertRight(	(float)(x+1)*hSpacing,	(float)heightMap[z*size+(x+1)]*vSpacing,(float)z*hSpacing);
			Vector3 tVertUp(	(float)x*hSpacing,		(float)heightMap[(z-1)*size+x]*vSpacing,(float)(z-1)*hSpacing);
			Vector3 tVertDown(	(float)x*hSpacing,		(float)heightMap[(z+1)*size+x]*vSpacing,(float)(z+1)*hSpacing);
			
			// get edge vectors
			Vector3 s1 = tVertLeft - tVert;
			Vector3 s2 = tVertUp - tVert;
			Vector3 s3 = tVertRight - tVert;
			Vector3 s4 = tVertDown - tVert;

			// get cross products (normals)
			Vector3 c1; c1.unitNormalOf(s1,s2);
			Vector3 c2; c2.unitNormalOf(s2,s3);
			Vector3 c3; c3.unitNormalOf(s3,s4);
			Vector3 c4; c4.unitNormalOf(s4,s1);
			
			// get average of normals
			Vector3 normal(0,0,0);
			normal = c1 + c2 + c3 + c4;
			normal /= 4.0f;
			normal.normalize();

			// record normal in map
			vNormalMap[z*size+x] = normal;
		}
	}

	// fix border vertices by setting them to the next row in
	// top row
	for (int i = 0; i < size; i++)			vNormalMap[i] = vNormalMap[i+size];
	// left row
	for (i = 0; i < size; i++)				vNormalMap[i*size] = vNormalMap[i*size+1];
	// bottom row
	for (i = size*(size-1); i < size; i++)	vNormalMap[i] = vNormalMap[i-size];
	// right row
	for (i = 0; i < size; i++)				vNormalMap[i*size+size-1] = vNormalMap[i*size+size-2];
}
*/
/*
void Terrain::fillIndexBuffer(int xOffset, int zOffset)
{
	unsigned short *indexPtr = indexBuffer;
	int startZIndex = zOffset * size;
	
	for (int z = 0; z < chunkNumVerts-1; ++z) {
		unsigned short index = startZIndex + xOffset;
		
		// if not first row add a degenerate triangle
		if (z > 0) {
			*indexPtr = index;
			++indexPtr;
		}

		for (int x = 0; x < chunkNumVerts; ++x) {
			*indexPtr = index;
			++indexPtr;

			*indexPtr = index+size;
			++indexPtr;
			
			// if end of row and not last row add a degenerate triangle
			if (x == chunkNumVerts-1 && z != chunkNumVerts-2) {
				*indexPtr = index+size;
				++indexPtr;
			}
			
			++index;
		}

		startZIndex += size;
	}
}
*/
/*
void Terrain::enableVertexBuffers(void) const
{
	glEnableClientState(GL_VERTEX_ARRAY);
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
//	glVertexPointer(3, GL_FLOAT, 0, (char *)0);
	
	glEnableClientState(GL_NORMAL_ARRAY);
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
//	glNormalPointer(GL_FLOAT, 0, (char *)0);
	
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, texBuffer0);
//	glTexCoordPointer(2, GL_FLOAT, 0, (char *)0);

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glBindBufferARB(GL_ARRAY_BUFFER_ARB, texBuffer1);
//	glTexCoordPointer(2, GL_FLOAT, 0, (char *)0);
}
*/
/*
void Terrain::disableVertexBuffers(void) const
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
*/
/*
void Terrain::buildVertexBuffers(void)
{
	// allocate space for the vertex data arrays
	float *vertexArray = new float[size*size*3];
	float *normalArray = new float[size*size*3];
	float *texArray0 = new float[size*size*2];
	float *texArray1 = new float[size*size*2];

	// fill the arrays with data
	for (int z = 0; z < size; ++z) {
		for (int x = 0; x < size; ++x) {
			// heightpoints
			vertexArray[z*(size*3)+(x*3)]	= x * hSpacing;
			vertexArray[z*(size*3)+(x*3)+1]	= (float)heightMap[z*size+x] * vSpacing;
			vertexArray[z*(size*3)+(x*3)+2]	= z * hSpacing;
			
			// normals
			normalArray[z*(size*3)+(x*3)]	= vNormalMap[z*size+x].x;
			normalArray[z*(size*3)+(x*3)+1]	= vNormalMap[z*size+x].y;
			normalArray[z*(size*3)+(x*3)+2]	= vNormalMap[z*size+x].z;

			// texture coordinate for the detail textures
			texArray0[z*(size*2)+(x*2)]		= x * hSpacing / texStretch;
			texArray0[z*(size*2)+(x*2)+1]	= z * hSpacing / texStretch;
			
			// texture coordinate for the layer map
			texArray1[z*(size*2)+(x*2)]		= x * hSpacing / mapLength;
			texArray1[z*(size*2)+(x*2)+1]	= z * hSpacing / mapLength;
		}
	}

	// create the VBOs
	glGenBuffersARB(1, &vertexBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vertexBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*size*3*sizeof(float), vertexArray, GL_STATIC_DRAW_ARB);
	glVertexPointer(3, GL_FLOAT, 0, (char *)0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glGenBuffersARB(1, &normalBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, normalBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*size*3*sizeof(float), normalArray, GL_STATIC_DRAW_ARB);
	glNormalPointer(GL_FLOAT, 0, (char *)0);
	glEnableClientState(GL_NORMAL_ARRAY);

	glGenBuffersARB(1, &texBuffer0);
	glClientActiveTextureARB(GL_TEXTURE0_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, texBuffer0);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*size*2*sizeof(float), texArray0, GL_STATIC_DRAW_ARB);
	glTexCoordPointer(2, GL_FLOAT, 0, (char *)0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glGenBuffersARB(1, &texBuffer1);
	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, texBuffer1);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, size*size*2*sizeof(float), texArray1, GL_STATIC_DRAW_ARB);
	glTexCoordPointer(2, GL_FLOAT, 0, (char *)0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	disableVertexBuffers();

	// delete local copies of the data
	delete [] vertexArray;
	delete [] normalArray;
	delete [] texArray0;
	delete [] texArray1;

	// create index buffer
	indexBufferSize = ((2*chunkNumVerts) + 2) * (chunkNumVerts-1) - 2;
	indexBuffer = new unsigned short[indexBufferSize];
}
*/
/*
void Terrain::destroyVertexBuffers(void)
{
	if (vertexBuffer != 0) {
		glDeleteBuffersARB(1, &vertexBuffer);
		vertexBuffer = 0;
	}
	if (normalBuffer != 0) {
		glDeleteBuffersARB(1, &normalBuffer);
		normalBuffer = 0;
	}
	if (texBuffer0 != 0) {
		glDeleteBuffersARB(1, &texBuffer0);
		texBuffer0 = 0;
	}
	if (texBuffer1 != 0) {
		glDeleteBuffersARB(1, &texBuffer1);
		texBuffer1 = 0;
	}

	delete [] indexBuffer;
}
*/
/*
bool Terrain::createHorizonTexture(void)
{	
	// find max terrain height on map to speed up algorithm
	float highestPoint = 0;
	for (int z = 0; z < size; ++z) {
		for (int x = 0; x < size; ++x) {
			if (heightMap[z*size+x] * vSpacing > highestPoint) highestPoint = heightMap[z*size+x] * vSpacing;
		}
	}
	
	// create the texture
	horizonMap = new int[horizonTexSize * horizonTexSize];
	
	float tSpacing = mapLength / (float)horizonTexSize;
	
	float locZ = tSpacing * 0.5f;
	for (int tz = 0; tz < horizonTexSize; ++tz) {
		
		float locX = tSpacing * 0.5f;
		for (int tx = 0; tx < horizonTexSize; ++tx) {
			horizonMap[tz*horizonTexSize+tx] = 0;

			// for each sun angle
			float angle = 10.0f;
			for (int a = 0; a < 32; ++a) {
				
				// set up the ray direction
				Vector3 rayDir(1.0f, 0.0f, 0.0f);
				rayDir.rot3D(0,0,math.degToIndex(angle));
				rayDir.normalize(); rayDir *= (hSpacing * 0.5f);

				// find ray starting place and increment once
				Vector3 ray(locX, getHeight(locX,locZ), locZ);
				ray += rayDir;
			
				// for each ray position
				while (	(ray.x < 0 || ray.x > mapLength) ||
						(ray.z < 0 || ray.z > mapLength) ||
						(ray.y > highestPoint)) {
				
					// check if ray intersects terrain
					if (ray.y <= getHeight(ray.x, ray.z)) {
						horizonMap[tz*horizonTexSize+tx] |= (1 << a);
						break;
					}
				
					// increment the ray
					ray += rayDir;
				}
				
				angle += 5.0f;
			}
			
			locX += tSpacing;
		}
		
		locZ += tSpacing;
	}

	// create the texture
//	int newTexID = texture.createFromData(	horizonMap, 4, textureSize, textureSize, GL_UNSIGNED_BYTE,
//											GL_CLAMP_TO_EDGE, GL_DECAL, false, true, true, "horizon map");

//	if (newTexID == -1) {
//		return false;
//	} else {
//		horizonMapTexID = newTexID;
		return true;
//	}
}
*/
