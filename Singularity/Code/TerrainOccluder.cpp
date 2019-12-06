/* ----==== TERRAINOCCLUDER.CPP ====---- */

#include <string>
#include "terrainoccluder.h"
#include "terrain.h"

//#include <GL\glew.h>	// temp
//#include "shadermanager.h"
#include "engine.h"

////////// class TerrainOccluder //////////

void TerrainOccluder::createRowMinBuffer(void)
{
	const Terrain &t = *terrainRef;

	// allocate it, table will have 1 more row than column
	rowMinBuffer = new float *[mapNumChunks + 1];
	for (int y = 0; y < mapNumChunks + 1; ++y)
		rowMinBuffer[y] = new float[mapNumChunks];

	// fill it
	for (int row = 0; row < mapNumChunks + 1; ++row)
	{
		for (int col = 0; col < mapNumChunks; ++col)
		{
			int min = 2147483647; //t.maxTerrainHeight; // set to max terrain height to start
			// starting and stopping vertex must be one over chunk boundary because of splines
			int vStart = (col == 0) ? 0 : -1;
			int vEnd = (col == mapNumChunks - 1) ? t.chunkNumVerts - 1 : t.chunkNumVerts;

			int vertIndex = (row * t.chunkSize * t.size) + (col * t.chunkSize) + vStart;
			//int vertIndex = (row*t.chunkSize*t.size) + (col*t.chunkSize);
			for (int vert = vStart; vert <= vEnd; ++vert)
			{
				//for (int vert = 0; vert < t.chunkNumVerts; ++vert) {
				if (t.heightMap[vertIndex] < min)
					min = t.heightMap[vertIndex];
				++vertIndex;
			}

			// index by vertex row then chunk column
			rowMinBuffer[row][col] = (float)min * t.vSpacing;
		}
	}
}

void TerrainOccluder::createColMinBuffer(void)
{
	const Terrain &t = *terrainRef;

	// allocate it, table will have 1 more column than row
	colMinBuffer = new float *[mapNumChunks];
	for (int y = 0; y < mapNumChunks; ++y)
		colMinBuffer[y] = new float[mapNumChunks + 1];

	// fill it
	for (int col = 0; col < mapNumChunks + 1; ++col)
	{
		for (int row = 0; row < mapNumChunks; ++row)
		{
			int min = 2147483647; //t.maxTerrainHeight; // set to max terrain height to start

			// starting and stopping vertex must be one over chunk boundary because of splines
			int vStart = (row == 0) ? 0 : -1; // protect against boundaries
			int vEnd = (row == mapNumChunks - 1) ? t.chunkNumVerts - 1 : t.chunkNumVerts;

			int vertIndex = ((row * t.chunkSize + vStart) * t.size) + (col * t.chunkSize);
			//int vertIndex = (row*t.chunkSize*t.size) + (col*t.chunkSize);
			for (int vert = vStart; vert <= vEnd; ++vert)
			{
				//for (int vert = 0; vert < t.chunkNumVerts; ++vert) {
				if (t.heightMap[vertIndex] < min)
					min = t.heightMap[vertIndex];
				vertIndex += t.size;
			}

			// index by chunk row then vertex column
			colMinBuffer[row][col] = (float)min * t.vSpacing;
		}
	}
}

void TerrainOccluder::createChunkTopBuffer(void)
{
	const Terrain &t = *terrainRef;

	chunkTopBuffer = new float[mapNumChunks * mapNumChunks];

	for (int r = 0; r < mapNumChunks; ++r)
	{
		int startZ = (r == 0) ? 0 : -1;
		int endZ = (r == mapNumChunks - 1) ? t.chunkNumVerts - 1 : t.chunkNumVerts;

		for (int c = 0; c < mapNumChunks; ++c)
		{
			int startX = (c == 0) ? 0 : -1;
			int endX = (c == mapNumChunks - 1) ? t.chunkNumVerts - 1 : t.chunkNumVerts;

			int max = -2147483647;
			int chunkIndex = (r * t.chunkSize * t.size) + (c * t.chunkSize);

			//for (int z = 0; z < t.chunkNumVerts; ++z) {
			for (int z = startZ; z <= endZ; ++z)
			{
				//for (int x = 0; x < t.chunkNumVerts; ++x) {
				for (int x = startX; x <= endX; ++x)
				{
					int vertIndex = chunkIndex + (z * t.size) + x;
					if (t.heightMap[vertIndex] > max)
						max = t.heightMap[vertIndex];
				}
			}

			chunkTopBuffer[r * mapNumChunks + c] = (float)max * t.vSpacing;
		}
	}
}

// p is an array of 5 in the order TL, TR, BL, BR, C
bool TerrainOccluder::checkQuadOccluded(const Vector3 &viewP, int viewChunkX, int viewChunkZ,
																				const Vector3 *p, int pChunkX, int pChunkZ) const
{
	const Terrain &t = *terrainRef;

	// get normalized vector from camera to each corner
	Vector3 diff[5];
	diff[0] = p[0];
	diff[0] -= viewP;
	diff[0].normalize(); // top left (0,0 being upper left)
	diff[1] = p[1];
	diff[1] -= viewP;
	diff[1].normalize(); // top right
	diff[2] = p[2];
	diff[2] -= viewP;
	diff[2].normalize(); // bottom left
	diff[3] = p[3];
	diff[3] -= viewP;
	diff[3].normalize(); // bottom right
	diff[4] = p[4];
	diff[4] -= viewP;
	diff[4].normalize(); // center

	// find 2 rays with largest span difference (lowest dot product) of the 4 corners
	float dot = 1.0f;
	int saveI = 0, saveJ = 0;
	for (int i = 0; i < 4; ++i)
	{
		for (int j = i + 1; j < 4; ++j)
		{
			float newDot = diff[i] * diff[j];
			if (newDot < dot)
			{
				dot = newDot;
				saveI = i;
				saveJ = j;
			}
		}
	}

	// set up fixed point ray to center and 2 corners with widest angle between
	const Vector3 *vPtr[3]; // line vectors
	vPtr[0] = &diff[4];			// do center first for performance
	vPtr[1] = &diff[saveI];
	vPtr[2] = &diff[saveJ];

	const Vector3 *pPtr[3]; // temp position references
	pPtr[0] = &p[4];
	pPtr[1] = &p[saveI];
	pPtr[2] = &p[saveJ];

	// cast those 3 rays
	int rayCount = 0;

	for (int p = 0; p < 3; p++)
	{
		if (fabs(vPtr[p]->z) > fabs(vPtr[p]->x))
		{ // if this is true it's a vertically oriented line, find horizontal (row) edges
			// find start row and and end row to check
			int startRow, endRow;
			if (vPtr[p]->z >= 0)
			{
				startRow = viewChunkZ + 1;
				endRow = pChunkZ;
			}
			else
			{
				startRow = viewChunkZ;
				endRow = pChunkZ + 1;
			}
			if (startRow < 0)
				startRow = 0;
			else if (startRow > mapNumChunks)
				startRow = mapNumChunks;

			// for each edge ray hits check against rowMinBuffer
			for (int row = startRow; row != endRow; row += (startRow <= endRow ? 1 : -1))
			{
				// find time step of ray
				float tStep = ((row * t.chunkLength) - viewP.z) / vPtr[p]->z;
				// find ray locations
				float rayX = viewP.x + (tStep * vPtr[p]->x);
				float rayY = viewP.y + (tStep * vPtr[p]->y);
				int col = (int)(rayX / t.chunkLength);
				if (col < 0 || col > mapNumChunks - 1)
					break;

				// if a ray is under the min edge height it is occluded
				if (rayY < rowMinBuffer[row][col])
				{
					rayCount++;

					/*					glColor4f(0,1,0,1);
					glBegin(GL_POINTS);
						glVertex3f(rayX, rayY, (row * t.chunkLength));
					glEnd();*/

					break;
				}

				/*				glColor4f(0,0,1,1);
				glBegin(GL_POINTS);
					glVertex3f(rayX, rayY, (row * t.chunkLength));
				glEnd();
				glColor4f(1,1,0,1);
				glBegin(GL_POINTS);
					glVertex3f(rayX, rowMinBuffer[row][col], (row * t.chunkLength));
				glEnd();*/
			}
		}
		else
		{ // horizontal line, find vertical (column) edges
			// find start col and and end col to check
			int startCol, endCol;
			if (vPtr[p]->x >= 0)
			{
				startCol = viewChunkX + 1;
				endCol = pChunkX;
			}
			else
			{
				startCol = viewChunkX;
				endCol = pChunkX + 1;
			}
			if (startCol < 0)
				startCol = 0;
			else if (startCol > mapNumChunks)
				startCol = mapNumChunks;

			// for each edge ray hits check against colMinBuffer
			for (int col = startCol; col != endCol; col += (startCol <= endCol ? 1 : -1))
			{
				// find time step of ray
				float tStep = ((col * t.chunkLength) - viewP.x) / vPtr[p]->x;
				// find ray locations
				float rayZ = viewP.z + (tStep * vPtr[p]->z);
				float rayY = viewP.y + (tStep * vPtr[p]->y);
				int row = (int)(rayZ / t.chunkLength);
				if (row < 0 || row > mapNumChunks - 1)
					break;

				// if a ray is under the min edge height it is occluded
				if (rayY < colMinBuffer[row][col])
				{
					rayCount++;

					/*					glColor4f(0,1,0,1);
					glBegin(GL_POINTS);
						glVertex3f((col * t.chunkLength), rayY, rayZ);
					glEnd();*/

					break;
				}
				/*				glColor4f(0,0,1,1);
				glBegin(GL_POINTS);
					glVertex3f((col * t.chunkLength), rayY, rayZ);
				glEnd();
				glColor4f(1,1,0,1);
				glBegin(GL_POINTS);
					glVertex3f((col * t.chunkLength), colMinBuffer[row][col], rayZ);
				glEnd();*/
			}
		}

		/*		glColor4f(1,0,0,1);
		glBegin(GL_LINES);
			glVertex3fv(viewP.v);
			glVertex3fv(pPtr[p]->v);
		glEnd();*/
	}

	// if all rays are occluded the cell is occluded
	if (rayCount == 3)
		return true;
	else
		return false;
}

void TerrainOccluder::performTerrainOcclusion(const Vector3 &viewP)
{
	///// TEMP
	/*	glUseProgramObjectARB(0);
	glDisable(GL_TEXTURE_2D);
	glPointSize(4.0f);
	glDisable(GL_POINT_SMOOTH);
	glDisable(GL_LINE_SMOOTH);*/
	/////

	const Terrain &t = *terrainRef;

	int viewChunkX = (int)(viewP.x / t.chunkLength);
	int viewChunkZ = (int)(viewP.z / t.chunkLength);

	// traverse front to back, skip cell player is in
	for (int c = 0; c < visibleCells.size(); ++c)
	{

		// find cell coordinates
		int chunkZ = visibleCells[c] / mapNumChunks;
		int chunkX = visibleCells[c] % mapNumChunks;
		if (chunkX != viewChunkX || chunkZ != viewChunkZ)
		{

			// find top of AABB for cell
			float top = chunkTopBuffer[chunkZ * mapNumChunks + chunkX];

			// find four top corners of cell
			float posX = chunkX * t.chunkLength;
			float posZ = chunkZ * t.chunkLength;
			Vector3 p[5];
			p[0].assign(posX, top, posZ);																								// top left
			p[1].assign(posX + t.chunkLength, top, posZ);																// top right
			p[2].assign(posX, top, posZ + t.chunkLength);																// bottom left
			p[3].assign(posX + t.chunkLength, top, posZ + t.chunkLength);								// bottom right
			p[4].assign(posX + t.chunkLength * 0.5f, top, posZ + t.chunkLength * 0.5f); // center

			///// TEMP - turn off occlusion
			if (engine.actuallyocclude)
				// check if quad is occluded and update occlusion map
				if (checkQuadOccluded(viewP, viewChunkX, viewChunkZ, p, chunkX, chunkZ))
					occlusionMap[visibleCells[c]] = 2;
			/////
		}
	}

	///// TEMP
	//	glUseProgramObjectARB(shader.getShaderID(0));
}

//-----------------------------------------------------------------------------

/*--== scanConvertLine ==------------------------------------------------------
	Use fixed point math for speed. Takes two floating-point 2D vectors and
	scan converts them storing values in *buffer[2]. Lines of a triangle must
	be oriented in a counter-clockwise order for proper edge detection.
-----------------------------------------------------------------------------*/
void TerrainOccluder::scanConvertLine(int x1, int y1, int x2, int y2)
{
	if ((y1 >> FIXEDSHIFT) == (y2 >> FIXEDSHIFT))
		return; // horizontal lines can be ignored (prevents / 0 error)

	if (y1 > y2)
	{ // left edge
		// add first point cell to buffer
		int cellX = x2 >> FIXEDSHIFT;
		int cellY = y2 >> FIXEDSHIFT;

		// start the line
		if (cellY >= mapNumChunks)
		{
			return; // this line is completely off the map
		}
		else if (cellY < -1)
		{
			cellY = -1; // start the line at the bottom of the map
		}
		else if (cellY >= 0 && cellX < buffer[cellY][0])
		{
			buffer[cellY][0] = cellX; // add starting point to buffer
		}

		int gradient = ((x2 - x1) << FIXEDSHIFT) / (y2 - y1);

		// find location to start line stepping
		int lineY = (cellY + 1) << FIXEDSHIFT; // first line above the cell
		int lineX = x2 + ((gradient * (lineY - y2)) >> FIXEDSHIFT);

		// add points in between
		while (lineY <= y1)
		{
			cellX = lineX >> FIXEDSHIFT;
			cellY = lineY >> FIXEDSHIFT;
			if (cellY > mapNumChunks)
				return; // we have reached the map boundary and can stop

			if (cellY < mapNumChunks)
				buffer[cellY][0] = cellX;
			if (cellY > 0 && cellX < buffer[cellY - 1][0])
				buffer[cellY - 1][0] = cellX;

			lineY += FLOATMULT;
			lineX += gradient;
		}

		// add last point cell to buffer
		cellX = x1 >> FIXEDSHIFT;
		cellY = y1 >> FIXEDSHIFT;
		if (cellY >= 0 && cellY < mapNumChunks)
		{
			if (cellX < buffer[cellY][0])
				buffer[cellY][0] = cellX;
		}
	}
	else
	{ // right edge

		// add first point cell to buffer
		int cellX = x1 >> FIXEDSHIFT;
		int cellY = y1 >> FIXEDSHIFT;

		// start the line
		if (cellY >= mapNumChunks)
		{
			return; // this line is completely off the map
		}
		else if (cellY < -1)
		{
			cellY = -1; // start the line at the bottom of the map
		}
		else if (cellY >= 0 && cellX > buffer[cellY][1])
		{
			buffer[cellY][1] = cellX; // add starting point to buffer
		}

		int gradient = ((x2 - x1) << FIXEDSHIFT) / (y2 - y1);

		// find location to start line stepping
		int lineY = (cellY + 1) << FIXEDSHIFT; // first line above the cell
		int lineX = x1 + ((gradient * (lineY - y1)) >> FIXEDSHIFT);

		// add points in between
		while (lineY <= y2)
		{
			cellX = lineX >> FIXEDSHIFT;
			cellY = lineY >> FIXEDSHIFT;
			if (cellY > mapNumChunks)
				return; // we have reached the map boundary and can stop

			if (cellY < mapNumChunks)
				buffer[cellY][1] = cellX;
			if (cellY > 0 && cellX > buffer[cellY - 1][1])
				buffer[cellY - 1][1] = cellX;

			lineY += FLOATMULT;
			lineX += gradient;
		}

		// add last point cell to buffer
		cellX = x2 >> FIXEDSHIFT;
		cellY = y2 >> FIXEDSHIFT;
		if (cellY >= 0 && cellY < mapNumChunks)
		{
			if (cellX > buffer[cellY][1])
				buffer[cellY][1] = cellX;
		}
	}
}

/*--== scanConvertTriangle ==--------------------------------------------------
	Vectors should be in map space, not world space. p0 is the viewpoint, and
	they must be arranged in counter-clockwise order.
-----------------------------------------------------------------------------*/
void TerrainOccluder::scanConvertTriangle(const Point2i &p0, const Point2i &p1, const Point2i &p2)
{
	// clear edge buffer
	for (int r = 0; r < mapNumChunks; r++)
	{
		buffer[r][0] = mapNumChunks;
		buffer[r][1] = -1;
	}

	// scan convert the edges and fill the buffer
	scanConvertLine(p0.x, p0.y, p1.x, p1.y);
	scanConvertLine(p1.x, p1.y, p2.x, p2.y);
	scanConvertLine(p2.x, p2.y, p0.x, p0.y);

	// find the top and bottom cells
	int cell0 = p0.y >> FIXEDSHIFT;
	int cell1 = p1.y >> FIXEDSHIFT;
	int cell2 = p2.y >> FIXEDSHIFT;
	int top = 0, bot = 0;
	if (cell0 >= cell1 && cell0 >= cell2)
		top = cell0;
	else if (cell1 >= cell0 && cell1 >= cell2)
		top = cell1;
	else
		top = cell2;
	if (cell0 <= cell1 && cell0 <= cell2)
		bot = cell0;
	else if (cell1 <= cell0 && cell1 <= cell2)
		bot = cell1;
	else
		bot = cell2;

	if (top >= mapNumChunks)
		top = mapNumChunks - 1;
	if (bot < 0)
		bot = 0;

	// set box variables Y component
	if (top > lastTop)
		lastTop = top;
	if (bot < lastBottom)
		lastBottom = bot;

	// set the visible cells of the occlusion map to 1
	unsigned char *location = occlusionMap + (bot * mapNumChunks);
	for (int r = bot; r <= top; r++)
	{
		int colStart = buffer[r][0];
		if (colStart < 0)
			colStart = 0;
		int colEnd = buffer[r][1];
		if (colEnd >= mapNumChunks)
			colEnd = mapNumChunks - 1;

		if (colEnd >= colStart)
			memset((location + colStart), 1, colEnd - colStart + 1);
		location += mapNumChunks;
	}
}

/*--== createVisibleList ==----------------------------------------------------

	Sets the variable list of visible cell indexes in front to back order
-----------------------------------------------------------------------------*/
void TerrainOccluder::createVisibleList(const Vector3 &lookDir)
{
	visibleCells.clear();

	if (lookDir.z >= 0)
	{
		if (lookDir.x >= 0)
		{ // up and right
			for (int r = lastTop; r >= lastBottom; r--)
			{
				for (int c = mapNumChunks - 1; c >= 0; c--)
				{
					if (occlusionMap[r * mapNumChunks + c] == 1)
						visibleCells.push_back(r * mapNumChunks + c);
				}
			}
		}
		else
		{ // up and left
			for (int r = lastTop; r >= lastBottom; r--)
			{
				for (int c = 0; c < mapNumChunks; c++)
				{
					if (occlusionMap[r * mapNumChunks + c] == 1)
						visibleCells.push_back(r * mapNumChunks + c);
				}
			}
		}
	}
	else
	{
		if (lookDir.x >= 0)
		{ // down and right
			for (int r = lastBottom; r <= lastTop; r++)
			{
				for (int c = mapNumChunks - 1; c >= 0; c--)
				{
					if (occlusionMap[r * mapNumChunks + c] == 1)
						visibleCells.push_back(r * mapNumChunks + c);
				}
			}
		}
		else
		{ // down and left
			for (int r = lastBottom; r <= lastTop; r++)
			{
				for (int c = 0; c < mapNumChunks; c++)
				{
					if (occlusionMap[r * mapNumChunks + c] == 1)
						visibleCells.push_back(r * mapNumChunks + c);
				}
			}
		}
	}
}

/*--== projectFrustumCone ==---------------------------------------------------

	Frustum points looking outward (0 is viewpoint, 1 is top-right, etc.)
			_______________
	(1)*-_           _-*(4)
	 |   - _(0)_ -   |
	 |     _ * _     |
	 | _ -       - _ |
	(2)*_______________*(3)

	Cone is used since it encompasses frustum rotation about the z axis (roll)
	Slant distance is used for projected straight-line distance since it
		encompasses view rotation about the x axis (pitch)
-----------------------------------------------------------------------------*/
void TerrainOccluder::scanConvertFrustum(const Vector3 &fp0, const Vector3 &fp1, const Vector3 &fp2,
																				 const Vector3 &fp3, const Vector3 &fp4, const Vector3 &lookDir)
{
	// scale points from world to unit coordinates
	float invMPL = 1.0f / terrainRef->chunkLength;
	// multiplier to convert 3d frustum points into 2d fixed point
	float mult = invMPL * FLOATMULT;

	// convert floating point into fixed point
	Point2i pi0((int)(fp0.x * mult), (int)(fp0.z * mult));
	Point2i pi1((int)(fp1.x * mult), (int)(fp1.z * mult));
	Point2i pi2((int)(fp2.x * mult), (int)(fp2.z * mult));
	Point2i pi3((int)(fp3.x * mult), (int)(fp3.z * mult));
	Point2i pi4((int)(fp4.x * mult), (int)(fp4.z * mult));

	// clear the occlusion map based on last frames box variables
	unsigned char *ocMapPos = occlusionMap + (lastBottom * mapNumChunks);
	for (int r = lastBottom; r <= lastTop; r++)
	{
		memset(ocMapPos, 0, mapNumChunks);
		ocMapPos += mapNumChunks;
	}

	// clear box variables
	lastBottom = mapNumChunks;
	lastTop = -1;

	// scan convert all faces of the frustum
	scanConvertTriangle(pi0, pi1, pi4); // top
	scanConvertTriangle(pi0, pi4, pi3); // right
	scanConvertTriangle(pi0, pi2, pi1); // left
	scanConvertTriangle(pi4, pi1, pi2); // far
	scanConvertTriangle(pi4, pi2, pi3); // far
	scanConvertTriangle(pi0, pi3, pi2); // bottom

	// build visible cell list
	createVisibleList(lookDir);

	// check for occlusion
	performTerrainOcclusion(fp0);

	// rebuild visible cell list
	createVisibleList(lookDir);
}

TerrainOccluder::TerrainOccluder(const Terrain *_terrainRef)
{
	terrainRef = _terrainRef;
	mapNumChunks = terrainRef->numChunks;

	// allocate the buffer for scan conversion
	buffer = new int *[mapNumChunks];
	for (int r = 0; r < mapNumChunks; r++)
	{
		buffer[r] = new int[2];
		buffer[r][0] = mapNumChunks;
		buffer[r][1] = -1;
	}

	// allocate the occlusion map
	occlusionMap = new unsigned char[mapNumChunks * mapNumChunks];
	memset(occlusionMap, 0, mapNumChunks * mapNumChunks);

	// init box variables
	lastBottom = mapNumChunks;
	lastTop = -1;

	FIXEDSHIFT = 12;
	FLOATMULT = 1 << FIXEDSHIFT;

	visibleCells.reserve(20);

	createRowMinBuffer();
	createColMinBuffer();
	createChunkTopBuffer();
}

TerrainOccluder::~TerrainOccluder()
{
	if (buffer)
	{
		for (int r = 0; r < mapNumChunks; r++)
			delete[] buffer[r];
		delete[] buffer;
	}

	if (rowMinBuffer)
	{
		for (int i = 0; i < mapNumChunks + 1; ++i)
			delete[] rowMinBuffer[i];
		delete[] rowMinBuffer;
	}
	if (colMinBuffer)
	{
		for (int i = 0; i < mapNumChunks; ++i)
			delete[] colMinBuffer[i];
		delete[] colMinBuffer;
	}

	delete[] occlusionMap;
	delete[] chunkTopBuffer;

	visibleCells.clear();
}
