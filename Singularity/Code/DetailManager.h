#ifndef DETAILMANAGER_H
#define DETAILMANAGER_H


class DetailLayerCell {
	private:
		int		cellX, cellZ;		// heightmap location of cell
		int		billboardCount;		// number of details planted

		// index buffer
		

		// vertex buffer
		
		
		// tex coord buffers

		

	public:
	
		explicit DetailLayerCell() {}
		~DetailLayerCell() {}
};


class DetailLayerGrid {
	private:
		DetailLayerCell		*layerGrid;


	public:
		

		explicit DetailLayerGrid() {}
		~DetailLayerGrid() {}
};


class DetailManager {
	private:
		

		unsigned int	*detailTexIDs;


	public:


		explicit DetailManager() {}
		~DetailManager() {}
};




#endif