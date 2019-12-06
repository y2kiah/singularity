/* ----==== TERRAINSHADOW.CPP ====---- */

#include <fstream>
#include "terrainshadow.h"
#include "terrain.h"
#include "texturemanager.h"
#include "MATHCODE\vector3.h"
#include "MATHCODE\spline.h"
#include "UTILITYCODE\lookupmanager.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class TerrainShadow //////////


void TerrainShadow::computeShadowMap(const Vector3 *dirToSun, float ambientSun, float ambientShade,
									 int texSize, Color3uc *shadowMap, int index)
{
	// cache all of the chunks by creating this fake cam, need lots of memory for this!
	Vector3 fakePos(0,0,0);
	Camera fakeCam(fakePos,0,0,0,60,terRef.mapLength,false);
	terRef.cacheChunks(fakeCam);

	// build buffer for max cell height
	float *maxCellHeight = new float[terRef.size*terRef.size];
	for (int z = 0; z < terRef.size; ++z) {
		for (int x = 0; x < terRef.size; ++x) {
			float hpBuffer[16];
			terRef.getBSplineControlPointsFromHeightmap(x,z,hpBuffer);
			maxCellHeight[z*terRef.size+x] = -terRef.maxTerrainHeight;
			for (int p = 0; p < 16; ++p) {
				if (hpBuffer[p] > maxCellHeight[z*terRef.size+x]) maxCellHeight[z*terRef.size+x] = hpBuffer[p];
			}
		}
	}

	// start stepping through the pixels
	const float STEPSCALE = 1.0f;
	float invHSpacing = 1.0f / terRef.hSpacing;
	float texelSpacing = terRef.mapLength / (float)texSize;
	Vector3 deltaCheck(dirToSun[index]); deltaCheck *= (texelSpacing * STEPSCALE);

	float posZ = 0;
	for (int v = 0; v < texSize; ++v) {
		float posX = 0;
		for (int u = 0; u < texSize; ++u) {
			// get values for current point			
			int cellX = (int)(posX / terRef.hSpacing);
			int cellZ = (int)(posZ / terRef.hSpacing);
			float tX = (posX - (cellX * terRef.hSpacing)) * invHSpacing;
			float tZ = (posZ - (cellZ * terRef.hSpacing)) * invHSpacing;
			CubicBSpline::setMiddleMatrixPtr(terRef.getCachedMatrixForCell(cellX,cellZ));
			Vector3 checkPos(posX, CubicBSpline::calcHeightOnPatchMatrix(tX, tZ, true), posZ);
			checkPos += deltaCheck;
			
			Vector3 normal(CubicBSpline::calcNormalOnPatchMatrix(tX, tZ, true));
			float slopeValue = normal * dirToSun[index];	// find dot product
			slopeValue = (slopeValue <= 0.0f) ? 0.0f : slopeValue;	// minumum of 0
			slopeValue = ambientShade + (slopeValue * (ambientSun-ambientShade));	// perpendicular and backfacing is ambient						

			// do the ray cast
			float shadeValue = ambientSun;

			while (checkPos.y <= terRef.maxTerrainHeight &&
					checkPos.x >= 0 && checkPos.x <= terRef.mapLength &&
					checkPos.z >= 0 && checkPos.z <= terRef.mapLength)
			{
				cellX = (int)(checkPos.x / terRef.hSpacing);
				cellZ = (int)(checkPos.z / terRef.hSpacing);

				// if ray is under top of bounding box
				if (checkPos.y <= maxCellHeight[cellZ*terRef.size+cellX]) {
					// compute cell changes
					tX = (checkPos.x - (cellX * terRef.hSpacing)) * invHSpacing;
					tZ = (checkPos.z - (cellZ * terRef.hSpacing)) * invHSpacing;
					CubicBSpline::setMiddleMatrixPtr(terRef.getCachedMatrixForCell(cellX,cellZ));

					// check if ray is under terrain
					if (checkPos.y < CubicBSpline::calcHeightOnPatchMatrix(tX, tZ, true)) {
						shadeValue = ambientShade;
						break;
					}
				}

				// increment
				checkPos += deltaCheck;				
			}

			shadeValue = (slopeValue < shadeValue) ? slopeValue : shadeValue;
			unsigned char shadeValueUC = shadeValue * 255.0f >= 255.0f ? 255 : (unsigned char)(shadeValue * 255.0f);

			int mapIndex = (v*texSize) + u;
			shadowMap[mapIndex].v[index]  = shadeValueUC;

			posX += texelSpacing;
		}
		posZ += texelSpacing;
	}
	
	delete [] maxCellHeight;
}


void TerrainShadow::blur(Color3uc *source, int width, int height, int radius)
{
	Color3uc *buffer = new Color3uc[width*height];

	// blur original image into buffer
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			Color3f total(0,0,0);
			for (int ky = -radius; ky <= radius; ++ky) {
				int useY = y + ky;
				if (useY < 0) useY = 0; else if (useY >= height) useY = height-1;
				
				for (int kx = -radius; kx <= radius; ++kx) {
					int useX = x + kx;
					if (useX < 0) useX = 0; else if (useX >= width) useX = width-1;

					total.r += source[(useY*width) + useX].r;
					total.g += source[(useY*width) + useX].g;
					total.b += source[(useY*width) + useX].b;
				}
			}
			float divisor = (float)(radius*2+1) * (float)(radius*2+1);
			buffer[y*width+x].r = (unsigned char)(total.r / divisor);
			buffer[y*width+x].g = (unsigned char)(total.g / divisor);
			buffer[y*width+x].b = (unsigned char)(total.b / divisor);
		}
	}

	// copy buffer into original image
	memcpy(source, buffer, width*height*3);
		
	delete [] buffer;
}


bool TerrainShadow::saveShadowMapToFile(const std::string &filename, int texSize, Color3uc *shadowMap)
{
	bool success = false;

	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	if (!outFile.is_open()) return false;
	
	outFile.seekp(0);
	outFile.write((const char *)(&texSize), sizeof(int));
	outFile.write((const char *)shadowMap, sizeof(Color3uc)*texSize*texSize);
	if (outFile.fail()) {
		console.addLineErr("     TerrainShadow: error writing to shadowmap file \"%s\"", filename.c_str());
	} else {
		console.addLine("     TerrainShadow: saved shadowmap file \"%s\"", filename.c_str());
		success = true;
	}

	outFile.close();

	return success;
}


/*----------------------------------------------------------------------------------------------------
void TerrainShadow::createShadowMap
	Expects array of 3 vectors in dirToSun, stores 1 shadow maps in each rgb component, avoid using
	alpha for 4th because of lossy compression
----------------------------------------------------------------------------------------------------*/
bool TerrainShadow::createShadowMap(const Vector3 *dirToSun, float ambientSun, float ambientShade,
									int texSize, const std::string &filename)
{
	bool success = false;

	Color3uc *shadowMap = new Color3uc[texSize*texSize];

	computeShadowMap(dirToSun, ambientSun, ambientShade, texSize, shadowMap, 0); // r
	computeShadowMap(dirToSun, ambientSun, ambientShade, texSize, shadowMap, 1); // g
	computeShadowMap(dirToSun, ambientSun, ambientShade, texSize, shadowMap, 2); // b
	blur(shadowMap, texSize, texSize, 1);
	blur(shadowMap, texSize, texSize, 1);
	success = saveShadowMapToFile(filename, texSize, shadowMap);

	delete [] shadowMap;

	return success;
}


bool TerrainShadow::loadFromRAW(const std::string &filename)
{
	int texSize = 0;
	shadowMapLoaded = false;

	std::ifstream inFile;
	inFile.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
	if (!inFile.is_open()) return false;
	
	inFile.seekg(0);
	inFile.read((char *)(&texSize), sizeof(int));

	if (texSize >= 1 && texSize <= 4096) {
		unsigned char *shadowMap = new unsigned char[texSize*texSize*3];
		
		inFile.read((char *)shadowMap, sizeof(unsigned char)*texSize*texSize*3);

		int texID = texture.createFromData(shadowMap, 3, texSize, texSize, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_DECAL,
											false, true, false, filename);
		if (texID != -1) {
			shadowMapTexID = texID;
			shadowMapLoaded = true;
			console.addLine("     terrain shadowmap texture loaded");
		} else {
			console.addLineErr("     terrain shadowmap texture failed to load");
		}

		delete [] shadowMap;
		
	} else { // if size is bad
		console.addLineErr("     terrain failed to read shadowmap of size %ix%i", texSize, texSize);
	}

	inFile.close();

	return shadowMapLoaded;
}


/*----------------------------------------------------------------------------------------------------
void TerrainShadow::createHorizonMap
----------------------------------------------------------------------------------------------------*/
bool TerrainShadow::createHorizonMap(int texSize, const std::string &filename)
{
	///// check size
	if (texSize <= 1 || texSize > 4096) {
		console.addLineErr("     TerrainShadow: horizonmap size %i out of range", texSize);	
		return false;
	}

	///// create horizon map
	unsigned char *horizonMap = new unsigned char[texSize * texSize * 2];

	float texStep = terRef.mapLength / (float)texSize;

	float mapZ = 0;
	for (int tz = 0; tz < texSize; ++tz) {
		
		float mapX = 0;
		for (int tx = 0; tx < texSize; ++tx) {
			int tIndex = (tz * texSize * 2) + (tx * 2);
			
			bool rayIntersect = false;

			// Get angle from east (+x, 0 to 90) and store in r component
			for (int angle = 255; angle >= 0; --angle) {
				
				Vector3 dirToSun(	math.getCos(static_cast<int>((float)math.ANGLE90 / 255.0f * (float)angle)),
									math.getSin(static_cast<int>((float)math.ANGLE90 / 255.0f * (float)angle)),
									0);
				dirToSun.normalize();

				Vector3 rayStep = dirToSun * texStep;
				Vector3 rayPos(mapX, terRef.getHeightBSpline(mapX, mapZ), mapZ);
				rayPos += rayStep; // get ray one step away from the terrain starting point
				
				while (rayPos.x < terRef.mapLength && rayPos.y < terRef.maxTerrainHeight) {
					
					if (rayPos.y <= terRef.getHeightBSpline(rayPos.x, rayPos.z)) {
						rayIntersect = true;
						horizonMap[tIndex] = angle;
						break;
					}					
					if (rayIntersect) break;
					
					rayPos += rayStep;
				}
				if (rayIntersect) break;
			}
			if (!rayIntersect) horizonMap[tIndex] = 0;

			// Get angle from west (-x, 90 to 180) and store in g component
			for (int angle2 = 255; angle2 >= 0; ++angle2) {
				
				Vector3 dirToSun(	math.getCos(math.ANGLE180 - static_cast<int>((float)math.ANGLE90 / 255.0f * (float)angle2)),
									math.getSin(math.ANGLE180 - static_cast<int>((float)math.ANGLE90 / 255.0f * (float)angle2)),
									0);
				dirToSun.normalize();

				Vector3 rayStep = dirToSun * texStep;
				Vector3 rayPos(mapX, terRef.getHeightBSpline(mapX, mapZ), mapZ);
				rayPos += rayStep; // get ray one step away from the terrain starting point
				
				while (rayPos.x > 0 && rayPos.y < terRef.maxTerrainHeight) {
					
					if (rayPos.y <= terRef.getHeightBSpline(rayPos.x, rayPos.z)) {
						rayIntersect = true;
						horizonMap[tIndex+1] = angle2;
						break;
					}					
					if (rayIntersect) break;
					
					rayPos += rayStep;
				}
				if (rayIntersect) break;
			}
			if (!rayIntersect) horizonMap[tIndex+1] = 0;

			mapX += texStep;
		}

		mapZ += texStep;
	}

	///// write to the file
	bool returnVal = true;

	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	if (outFile.is_open()) {

		outFile.seekp(0);
		outFile.write((const char *)(&texSize), sizeof(int));
		outFile.write((const char *)horizonMap, texSize*texSize*2);

		if (outFile.fail()) {
			console.addLineErr("     TerrainShadow: error writing to horizonmap file \"%s\"", filename.c_str());
			returnVal = false;
		}

		outFile.close();

	} else {
		console.addLineErr("     TerrainShadow: unable to open horizonmap file \"%s\" for output", filename.c_str());
		returnVal = false;
	}

	///// clean up
	delete [] horizonMap;

	console.addLine("     TerrainShadow: horizonmap \"%s\" created %ix%i", filename.c_str(), texSize, texSize);
	return returnVal;
}


bool TerrainShadow::loadHorizonMap(const std::string &filename)
{
	int texSize = 0;
	horizonMapLoaded = false;

	std::ifstream inFile;
	inFile.open(filename.c_str(), std::ios_base::binary | std::ios_base::in);
	if (!inFile.is_open()) return false;
	
	inFile.seekg(0);
	inFile.read((char *)(&texSize), sizeof(int));

	if (texSize >= 1 && texSize <= 4096) {
		unsigned char *horizonBuffer = new unsigned char[texSize*texSize*2];
		unsigned char *horizonMap = new unsigned char[texSize*texSize*3];
		
		inFile.read((char *)horizonBuffer, sizeof(unsigned char)*texSize*texSize*2);
		for (int c = 0; c < texSize*texSize; ++c) {
			horizonMap[c*3] = horizonBuffer[c*2];
			horizonMap[c*3+1] = horizonBuffer[c*2+1];
			horizonMap[c*3+2] = 0;
		}

		int texID = texture.createFromData(horizonMap, 3, texSize, texSize, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_DECAL,
											false, false, false, filename);
		if (texID != -1) {
			horizonMapTexID = texID;
			horizonMapLoaded = true;
			console.addLine("     terrain horizonmap texture loaded \"%s\"", filename.c_str());
		} else {
			console.addLineErr("     terrain horizonmap texture failed to load \"%s\"", filename.c_str());
		}

		delete [] horizonBuffer;
		delete [] horizonMap;
		
	} else { // if size is bad
		console.addLineErr("     terrain failed to read horizonmap of size %ix%i", texSize, texSize);
	}

	inFile.close();

	return horizonMapLoaded;
}


void TerrainShadow::createNormalMap(int texSize)
{
	// check size
	if (texSize <= 1 || texSize > 4096) {
		console.addLineErr("     terrain normalmap size %i out of range", texSize);	
		return;
	}
	
	unsigned char *normalMap = new unsigned char[texSize * texSize * 3];

	const float texStep = terRef.mapLength / (float)texSize;

	// create height and normal maps
		
	float zPos = texStep * 0.5f;
	for (int z = 0; z < texSize; ++z) {

		float xPos = texStep * 0.5f;
		for (int x = 0; x < texSize; ++x) {
			
			Vector3 n(terRef.getNormalBSpline(xPos, zPos));
			
			normalMap[(z*texSize*3)+(x*3)]   = static_cast<unsigned char>((n.x+1.0f) * 127.5f);
			normalMap[(z*texSize*3)+(x*3)+1] = static_cast<unsigned char>((n.y+1.0f) * 127.5f);
			normalMap[(z*texSize*3)+(x*3)+2] = static_cast<unsigned char>((n.z+1.0f) * 127.5f);

			xPos += texStep;
		}
		zPos += texStep;
	}

	// write to the file
/*	bool returnVal = true;

	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
	if (outFile.is_open()) {

		outFile.seekp(0);
		outFile.write((const char *)(&texSize), sizeof(int));
		outFile.write((const char *)normalMap, texSize*texSize*3);

		if (outFile.fail()) {
			console.addLineErr("     TerrainShadow: error writing to normal map file \"%s\"", filename.c_str());
			returnVal = false;
		}

		outFile.close();

	} else {
		console.addLineErr("     TerrainShadow: unable to open file \"%s\" for output", filename.c_str());
		returnVal = false;
	}*/

	// send to OpenGL
	int texID = texture.createFromData(normalMap, 3, texSize, texSize, GL_UNSIGNED_BYTE, GL_CLAMP_TO_EDGE, GL_DECAL,
										false, true, false, "normalMap");
	if (texID != -1) {
		normalMapTexID = texID;
		normalMapLoaded = true;
		console.addLine("     terrain normalmap texture created %ix%i", texSize, texSize);
	} else {
		console.addLineErr("     terrain normalmap texture creation failed");
	}

	// clean up
	delete [] normalMap;
}


TerrainShadow::TerrainShadow(/*const */Terrain *ter) : terRef(*ter)
{
	shadowMapTexID = -1;
	shadowMapLoaded = false;

	horizonMapTexID = -1;
	horizonMapLoaded = false;

	normalMapTexID = -1;
	normalMapLoaded = false;
	createNormalMap(1024);
}
