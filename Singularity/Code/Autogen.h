/* ----==== AUTOGEN.H ====---- */

#ifndef AUTOGEN_H
#define AUTOGEN_H

/*------------------
---- STRUCTURES ----
------------------*/

class DetailPattern {
	private:
		// here put all object coord info, planes etc


	public:

		explicit DetailPattern();
		~DetailPattern();
};

class DetailInstance {
	private:
		unsigned int	pattern_id;	// index to the detail pattern
		unsigned int	grid_loc;	// grid location of the upper left corner (0,0,0) of the object
		unsigned int	grid_size;	// number of elements the object covers on the terrain
		
		float			anim_x_offset;	// offset to the x coordinate of the upper grass coordinates

	public:

		void	UpdateAnimation(void);
		void	DrawOnTerrain(void) const;

		explicit DetailInstance();
		~DetailInstance();
};



#endif