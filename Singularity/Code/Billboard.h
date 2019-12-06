/* ----==== BILLBOARD.H ====---- */

#ifndef BILLBOARD_H
#define BILLBOARD_H

/*---------------
---- DEFINES ----
---------------*/

enum BBoardType { X=0,Y,Z,XY,XZ,YZ,XYZ };

/*------------------
---- STRUCTURES ----
------------------*/

class Billboard_Instance {
	private:
		Point3D			wp;
		float			hsize, vsize;
		unsigned int	texture_id;

		BBoardType		type;

	public:
		void Render(void) const;

		explicit Billboard_Instance() {}
};

#endif