/* ----==== FRUSTUM.CPP ====---- */

#include <GL\glew.h>
#include <cmath>
#include "frustum.h"
#include "options.h"
#include "engine.h"
#include "UTILITYCODE\lookupmanager.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class Frustum //////////

colTestType Frustum::sphereInsidePlanes(const BSphere &_sphere) const
{
	colTestType returnval = COLLISION_FULL_INSIDE;

	for (int c = 0; c < 6; c++) {
		colTestType thisreturn = _sphere.collideWithPlane(side[c]);
		if (thisreturn == COLLISION_FULL_INSIDE) return COLLISION_FALSE;	// the frustum planes are turned inside out, positive dist on the inside
		if (thisreturn == COLLISION_TRUE) returnval = COLLISION_TRUE;		// so these look weird but work
	}

	return returnval;
}


bool Frustum::checkPointIn(const Vector3 &p) const
{
	for (int c = 0; c < 6; c++) if (side[c].findDist(p) < 0) return false;
	return true;
}


void Frustum::calcWorldPoints(const Vector3 &p, int zangle, int xangle, int yangle)
{
	wp[0] = p;									// Set first world point to new world position

	for (int c = 1; c < 5; ++c) wp[c] = op[c];	// Reset world points to original points

	for (int c2 = 1; c2 < 5; ++c2) {			// Rotate and translate all points but first
		wp[c2].rot3D(xangle,yangle,zangle);
		wp[c2] += p;
	}

	// get the look direction
	lookDir.normalOf(wp[4]-wp[2], wp[1]-wp[2]);

	wSphere = oSphere;
	wSphere.p.rot3D(xangle,yangle,zangle);
	wSphere.p += p;

	side[0].set(wp[0], wp[2], wp[1]);	// left
	side[1].set(wp[0], wp[4], wp[3]);	// right
	side[2].set(wp[0], wp[1], wp[4]);	// top
	side[3].set(wp[0], wp[3], wp[2]);	// bottom
	side[4].set(wp[1], wp[3], wp[4]);	// far
	side[5].set(wp[0], -side[4].n);		// near
}


void Frustum::draw(void) const
{
	// For camera view
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glBegin(GL_LINES);
		glColor3f(1,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(1,0,0);
		glVertex3f(wp[1].x,wp[1].y,wp[1].z);
		glColor3f(1,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(1,0,0);
		glVertex3f(wp[2].x,wp[2].y,wp[2].z);
		glColor3f(1,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(1,0,0);
		glVertex3f(wp[3].x,wp[3].y,wp[3].z);
		glColor3f(1,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(1,0,0);
		glVertex3f(wp[4].x,wp[4].y,wp[4].z);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glColor3f(1,0,0);
		glVertex3f(wp[1].x,wp[1].y,wp[1].z);
		glVertex3f(wp[2].x,wp[2].y,wp[2].z);
		glVertex3f(wp[3].x,wp[3].y,wp[3].z);
		glVertex3f(wp[4].x,wp[4].y,wp[4].z);
		glVertex3f(wp[1].x,wp[1].y,wp[1].z);
	glEnd();

	glBegin(GL_LINES);
		glColor3f(0,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(0,0,1);
		glVertex3f(wp[0].x+side[0].n.x*5,wp[0].y+side[0].n.y*5,wp[0].z+side[0].n.z*5);

		glColor3f(0,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(0,0,1);
		glVertex3f(wp[0].x+side[1].n.x*5,wp[0].y+side[1].n.y*5,wp[0].z+side[1].n.z*5);

		glColor3f(0,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(0,0,1);
		glVertex3f(wp[0].x+side[2].n.x*5,wp[0].y+side[2].n.y*5,wp[0].z+side[2].n.z*5);

		glColor3f(0,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(0,0,1);
		glVertex3f(wp[0].x+side[3].n.x*5,wp[0].y+side[3].n.y*5,wp[0].z+side[3].n.z*5);

		glColor3f(0,1,0);
		glVertex3f(wp[0].x,wp[0].y,wp[0].z);
		glColor3f(0,0,1);
		glVertex3f(wp[0].x+side[4].n.x*5,wp[0].y+side[4].n.y*5,wp[0].z+side[4].n.z*5);
	glEnd();

	glBegin(GL_POINTS);
		glColor3ub(255,0,0);
		glVertex3fv(wSphere.p.v);
	glEnd();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}


void Frustum::resetFrustum(float angle, float viewDist)
{
	// Camera Point
	op[0].assign(0,0,0);

	// Upper Left
	op[1].assign(-tanf(angle * DEGTORAD * 0.5f) * viewDist * ((float)gOptions.RESX / (float)gOptions.RESY),
				tanf(angle * DEGTORAD * 0.5f) * viewDist,
				-viewDist);
	// Lower Left
	op[2].assign(op[1].x, -op[1].y, -viewDist);

	// Lower Right
	op[3].assign(-op[1].x, -op[1].y, -viewDist);

	// Upper Right
	op[4].assign(-op[1].x, op[1].y, -viewDist);
	
	for (int p = 0; p < 5; p++) wp[p] = op[p];


	// Make the side planes of the frustrum
	side[0].set(wp[0], wp[2], wp[1]);	// left
	side[1].set(wp[0], wp[4], wp[3]);	// right
	side[2].set(wp[0], wp[1], wp[4]);	// top
	side[3].set(wp[0], wp[3], wp[2]);	// bottom
	side[4].set(wp[1], wp[3], wp[4]);	// far
	side[5].set(wp[0], -side[4].n);		// near

	// Find center of sphere and radius
	oSphere.p.x = 0;
	oSphere.p.y = 0;
	
	Vector3 midpoint;
	midpoint += op[0];
	midpoint += op[1];
	midpoint /= 2.0f;
	const float xdiff = 0 - midpoint.x;
	const float xstep = xdiff / side[0].n.x;
	oSphere.p.z = midpoint.z + (side[0].n.z * xstep);

	const float ydiff = 0 - midpoint.y;
	const float ystep = ydiff / side[2].n.y;
	oSphere.p.z = oSphere.p.z + side[2].n.z * ystep;
	
	oSphere.radius = oSphere.p.dist(op[0]) + 1.0f;	// add 1.0 for slight inaccuracies

	// find distance from origin to a corner
	distToCorner = op[0].dist(op[1]);
}


Frustum::Frustum(float angle, float viewDist)
{
	resetFrustum(angle,viewDist);
}


/*Frustum::Frustum(const Vector3 &o, const Vector3 &ul, const Vector3 &ll, const Vector3 &lr, const Vector3 &ur)
{
	// this constructor could eventually be used for occluder objects and portals

	// Camera Point
	op[0] = o;
	// Upper Left
	op[1] = ul;
	// Lower Left
	op[2] = ll;
	// Lower Right
	op[3] = lr;
	// Upper Right
	op[4] = ur;
	
	for (int p = 0; p < 5; p++) wp[p] = op[p];
	

	// Make the side planes of the frustrum
	side[0].set(wp[0], wp[2], wp[1]);	// left
	side[1].set(wp[0], wp[4], wp[3]);	// right
	side[2].set(wp[0], wp[1], wp[4]);	// top
	side[3].set(wp[0], wp[3], wp[2]);	// bottom
	side[4].set(wp[1], wp[3], wp[4]);	// far
//	side[5].set(wp[0], wp[0], wp[0]);	// near
//	side[5].n.x = -side[4].n.x;
//	side[5].n.y = -side[4].n.y;
//	side[5].n.z = -side[4].n.z;


	// Do not set up a sphere for this type of frustrum
	oSphere.p.assign(0,0,0);
	oSphere.radius = 0;
}*/