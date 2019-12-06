/* ----==== TERRAINOCCLUDER.H ====---- */

#ifndef TERRAINOCCLUDER_H
#define TERRAINOCCLUDER_H

#include <vector>
#include "MATHCODE\vector2.h"
#include "MATHCODE\vector3.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Terrain;


class TerrainOccluder {
	private:
		///// STRUCTURES
		struct OcclusionRay {
			int x, z;
			float y;
		};

		///// ATTRIBUTES

		// For general purpose and frustum culling
		const Terrain *		terrainRef;			// should reference the terrain that created the instance

		int					mapNumChunks;		// 2^n

		int					lastBottom;			// keeps record of top and bottom row from last frame
		int					lastTop;			// for fast clearing

		int					**buffer;			// buffer to store left and right values during scan conversion
		unsigned char		*occlusionMap;		// the occlusion map, 1 is visible, 0 is not visible, 2 is terrain occluded
		std::vector<int>	visibleCells;		// variable list of visible cells by index, front to back

		// Fixed point math variables
		int					FIXEDSHIFT;			// value to go from fixed point coords to cell units
		int					FLOATMULT;			// 2^FIXEDSHIFT (or 1 << FIXEDSHIFT)
												//		Value to go from world coords to fixed point coords
												//		This is also the length of 1 unit in fixed point coords
		
		// For occlusion culling
		float				**rowMinBuffer;		// for each cell boundary row (numChunks+1)
												// store lowest of (numChunks) along that row
		float				**colMinBuffer;
		float				*chunkTopBuffer;	// store highest point per chunk	


		///// METHODS

		// For occlusion culling
		void		createRowMinBuffer(void);
		void		createColMinBuffer(void);
		void		createChunkTopBuffer(void);
		void		performTerrainOcclusion(const Vector3 &viewP);

		// For frustum culling
		void		scanConvertLine(int x1, int y1, int x2, int y2);
		void		scanConvertTriangle(const Point2i &p0, const Point2i &p1, const Point2i &p2);

		void		createVisibleList(const Vector3 &lookDir);	// create list in front to back order

	public:

		bool		checkQuadOccluded(	const Vector3 &viewP, int viewChunkX, int viewChunkZ,
										const Vector3 *p, int pChunkX, int pChunkZ) const;

		void		scanConvertFrustum(	const Vector3 &fp0, const Vector3 &fp1, const Vector3 &fp2,
										const Vector3 &fp3, const Vector3 &fp4, const Vector3 &lookDir);

		const unsigned char	*		getOcclusionMap() const { return occlusionMap; }
		const std::vector<int> &	getVisibleList()  const	{ return visibleCells; }

		explicit TerrainOccluder(const Terrain *_terrainRef);
		~TerrainOccluder();
};

#endif
