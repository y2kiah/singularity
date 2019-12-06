/* ----==== CAMERA.H ====---- */

#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include "texcoord.h"
#include "frustum.h"
#include "MATHCODE\vector3.h"


/*---------------
---- DEFINES ----
---------------*/

#define		MAX_TRANSPARENT_TRIS	10000


/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class CameraMgr_CAccess;


struct TransparentTri {
	Vector3		vert[3];	// world space coordinates
	TexCoord	tCoord[3];	// texture coordinates
	int			texID;		// index into the TexList
	float		distToCam;	// distance to the camera, for sorting
};


class TransparentTriList {
	private:
		TransparentTri	trilist[MAX_TRANSPARENT_TRIS];
		TransparentTri	**triarray;		// array of pointers to the above list, for rendering order
		float			*distarray;		// list of distances to be sorted
		int				listsize;		// size of the above list
		int				lastlistsize;	// list size last frame

		void fillArrays(void);
		void performInsertionSort(void);
//		void performQuickSort(const int left,const int right);

	public:
		void push(TransparentTri &_newtri);
		void render(void);

		explicit TransparentTriList() : triarray(0), distarray(0), listsize(0), lastlistsize(0) {}
		~TransparentTriList();
};


class Camera {
	friend class CameraMgr_CAccess;

	private:
		
		int				yAngle, xAngle, zAngle;
		float			viewAngle, viewDist;
		float			zoom;

		bool			drawFog;
		bool			renderScene;	// hides or displays the scene

		Vector3			p;
		Frustum			frustum;

		TransparentTriList	transTris;

	public:

		///// Accessors

		const Vector3		&getP() const			{ return p; }
		int					getXAngle() const		{ return xAngle; }
		int					getYAngle() const		{ return yAngle; }
		int					getZAngle() const		{ return zAngle; }
		float				getViewDist() const		{ return viewDist; }
		float				getZoom() const			{ return zoom; }
		bool				getRenderScene() const	{ return renderScene; }
		const Frustum		&getFrustum() const		{ return frustum; }
		TransparentTriList	&getTransTris()			{ return transTris; }

		///// Mutators

		void setPos(const Vector3 &_p);
		void setPosIncrement(const Vector3 &_p);
		void setOrientation(int _xa, int _ya, int _za);
		void setOrientationIncrement(int _xa, int _ya, int _za);
		void setAngle(float a);
		void setViewDist(float d);
		void setZoom(float z);
		void zoomDouble(void) { setZoom(zoom*2.0f); }
		void zoomHalf(void) { setZoom(zoom*0.5f); }
		
		///// Other

		void show(void) const;
		void renderWorld(void);

		///// temp
		void setCameraSpaceMatrix() const;
		/////

		///// Constructors / Destructor

		explicit Camera(const Vector3 &_p, int _xa, int _ya, int _za, float _va, float _vd, bool _fog);
		~Camera();
};

#endif
