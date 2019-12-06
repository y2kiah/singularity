/* ----==== WATER.CPP ====---- */

#include "water.h"
#include "UTILITYCODE\msgassert.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Water //////////
int lookNeighbors(int x, int z, unsigned char *fillMap, int w, int h, const unsigned char *hMap, int fillHeight);

void fillWaterMap(int x, int z, unsigned char *fillMap, int w, int h, const unsigned char *hMap, int fillHeight)
{
	msgAssert(x < 0 && x >= w && z < 0 && z >= h, "fillWaterMap: out of bounds"); // error check

	int lowX = x, highX = x, lowZ = z, highZ = z;
	bool cont = true, doLowZ = true, doHighZ = true, doLowX = true, doHighX = true;

	while (cont) {
		// fill the outer ring
		if (lowZ >= 0 && doLowZ) {
			doLowZ = false;
			for (int p = lowX; p <= highX; ++p) {
				fillMap[lowZ*h+p] = lookNeighbors(p,lowZ,fillMap,w,h,hMap,fillHeight);
				if (fillMap[lowZ*h+p] == 1) doLowZ = true; // if any cell in the row was 1 then do next row
			}
		}

		if (highZ < h && doHighZ) {
			doHighZ = false;
			for (int p = lowX; p <= highX; ++p) {
				fillMap[highZ*h+p] = lookNeighbors(p,highZ,fillMap,w,h,hMap,fillHeight);
				if (fillMap[highZ*h+p] == 1) doHighZ = true;
			}
		}

		if (lowX >= 0 && doLowX) {
			doLowX = false;
			for (int p = lowZ; p <= highZ; ++p) {
				fillMap[p*h+lowX] = lookNeighbors(lowX,p,fillMap,w,h,hMap,fillHeight);
				if (fillMap[p*h+lowX] == 1) doLowX = true;
			}
		}

		if (highX < w && doHighX) {
			doHighX = false;
			for (int p = lowZ; p <= highZ; ++p) {
				fillMap[p*h+highX] = lookNeighbors(highX,p,fillMap,w,h,hMap,fillHeight);
				if (fillMap[p*h+highX] == 1) doHighX = true;
			}
		}

		// increase ring size and decide if filling should continue
		cont = false;
		lowX -= lowX > 0 ? 1 : 0;
		highX += highX < w-1 ? 1 : 0;
		lowZ -= lowZ > 0 ? 1 : 0;
		highZ += highZ < h-1 ? 1 : 0;
		if (lowZ >= 0 || highZ < h || lowX >= 0 || highX < w) cont = true;
	}
}

int lookNeighbors(int x, int z, unsigned char *fillMap, int w, int h, const unsigned char *hMap, int fillHeight)
{
	if (fillHeight <= hMap[z*h+x]) return 0; // if water level is lower than terrain level return 0

	int lowX = x-1<0 ? 0 : x-1;
	int highX = x+1>=w ? w-1 : x+1;
	int lowZ = z-1<0 ? 0 : z-1;
	int highZ = z+1>=h ? h-1 : z+1;

	for (int s = lowX; s <= highZ; ++s) {
		for (int t = lowZ; t <= highZ; ++t) {
			if (fillMap[t*h+s] == 1) return 1; // if any neighbor has water return 1
		}
	}

	return 0; // otherwise return 0
}
