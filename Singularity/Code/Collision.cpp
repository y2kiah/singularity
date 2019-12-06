/* ----==== COLLISION.CPP ====---- */

#include <GL\glew.h>
#include "collision.h"
#include "frustum.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class colSphere //////////

colTestType BSphere::collideWithPoint(const Vector3 &_p) const
{
	if (p.distSquared(_p) >= radius*radius) return COLLISION_FALSE;
	else return COLLISION_TRUE;
}


colTestType BSphere::collideWithPlane(const Plane3 &_pl) const
{
	const float dist = _pl.findDist(p);
	if (dist < -radius) return COLLISION_FULL_INSIDE;
	if (dist <= radius) return COLLISION_TRUE;
	else return COLLISION_FALSE;
}


colTestType BSphere::collideWithSphere(const BSphere &_sp) const
{
	const float dist = p.distSquared(_sp.p);
	if (dist >= ((radius + _sp.radius)*(radius + _sp.radius))) return COLLISION_FALSE;
	if (dist <= ((radius - _sp.radius)*(radius - _sp.radius))) return COLLISION_FULL_INSIDE;
	else return COLLISION_TRUE;
}


////////// class colAABBi //////////

colTestType	colAABBi::collideWithPoint(const Vector3 &_p) const
{
	if (_p.x >= xpos[0] && _p.x <= xpos[1] && _p.y >= ypos[0] && _p.y <= ypos[1] && _p.z >= zpos[0] && _p.z <= zpos[1])
		return COLLISION_TRUE;
	else
		return COLLISION_FALSE;
}


colTestType colAABBi::collideWithFrustum(const Frustum &_f) const
{
	// Accurate bounding box tests
	const Plane3 *side = &_f.getSides();
	Vector3 usep;
	int c2 = 0;

	const float fxpos[2] = {(float)xpos[0],(float)xpos[1]};
	const float fypos[2] = {(float)ypos[0],(float)ypos[1]};
	const float fzpos[2] = {(float)zpos[0],(float)zpos[1]};

	for (int p = 0; p < 6; p++) {
		int c1 = 0;

		usep.assign(fxpos[0], fypos[0], fzpos[0]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[1], fypos[0], fzpos[1]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[0], fypos[0], fzpos[1]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[1], fypos[0], fzpos[0]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		if (c1 == 0) return COLLISION_FALSE;
		else if (c1 == 8) c2++;
	}

	if (c2 == 6) return COLLISION_FULL_INSIDE;
	else return COLLISION_TRUE;
}


void colAABBi::draw(float r, float g, float b) const
{
	glDisable(GL_TEXTURE_2D);
	glColor3f(r,g,b);
	glBegin(GL_LINE_STRIP);
		glVertex3i(xpos[0],ypos[0],zpos[0]);
		glVertex3i(xpos[1],ypos[0],zpos[0]);
		glVertex3i(xpos[1],ypos[0],zpos[1]);
		glVertex3i(xpos[0],ypos[0],zpos[1]);
		glVertex3i(xpos[0],ypos[0],zpos[0]);
	glEnd();

	glBegin(GL_LINE_STRIP);
		glVertex3i(xpos[0],ypos[1],zpos[0]);
		glVertex3i(xpos[1],ypos[1],zpos[0]);
		glVertex3i(xpos[1],ypos[1],zpos[1]);
		glVertex3i(xpos[0],ypos[1],zpos[1]);
		glVertex3i(xpos[0],ypos[1],zpos[0]);
	glEnd();

	glBegin(GL_LINES);
		glVertex3i(xpos[0],ypos[0],zpos[0]);
		glVertex3i(xpos[0],ypos[1],zpos[0]);
		
		glVertex3i(xpos[1],ypos[0],zpos[0]);
		glVertex3i(xpos[1],ypos[1],zpos[0]);

		glVertex3i(xpos[1],ypos[0],zpos[1]);
		glVertex3i(xpos[1],ypos[1],zpos[1]);

		glVertex3i(xpos[0],ypos[0],zpos[1]);
		glVertex3i(xpos[0],ypos[1],zpos[1]);
	glEnd();	

	glEnable(GL_TEXTURE_2D);
}


colAABBi::colAABBi(int x1, int x2, int y1, int y2, int z1, int z2)
{
	// order them from lowest to highest
	if (x1 <= x2) {
		xpos[0] = x1;
		xpos[1] = x2;
	} else {
		xpos[1] = x1;
		xpos[0] = x2;
	}
	
	if (y1 <= y2) {
		ypos[0] = y1;
		ypos[1] = y2;
	} else {
		ypos[1] = y1;
		ypos[0] = y2;
	}

	if (z1 <= z2) {
		zpos[0] = z1;
		zpos[1] = z2;
	} else {
		zpos[1] = z1;
		zpos[0] = z2;
	}
}


////////// class colAABBf //////////

colTestType	colAABBf::collideWithPoint(const Vector3 &_p) const
{
	if (_p.x >= xpos[0] && _p.x <= xpos[1] && _p.y >= ypos[0] && _p.y <= ypos[1] && _p.z >= zpos[0] && _p.z <= zpos[1])
		return COLLISION_TRUE;
	else
		return COLLISION_FALSE;
}


colTestType colAABBf::collideWithFrustum(const Frustum &_f) const
{
	// Accurate bounding box tests
	const Plane3 *side = &_f.getSides();
	Vector3 usep;
	int c2 = 0;

	const float fxpos[2] = {xpos[0], xpos[1]};
	const float fypos[2] = {ypos[0], ypos[1]};
	const float fzpos[2] = {zpos[0], zpos[1]};

	for (int p = 0; p < 6; p++) {
		int c1 = 0;

		usep.assign(fxpos[0], fypos[0], fzpos[0]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[1], fypos[0], fzpos[1]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[0], fypos[0], fzpos[1]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		usep.assign(fxpos[1], fypos[0], fzpos[0]);
		if (side[p].findDist(usep) > 0) c1++;

		usep.y = fypos[1];
		if (side[p].findDist(usep) > 0) c1++;

		if (c1 == 0) return COLLISION_FALSE;
		else if (c1 == 8) c2++;
	}

	if (c2 == 6) return COLLISION_FULL_INSIDE;
	else return COLLISION_TRUE;
}


colAABBf::colAABBf(Vector3 &ul,Vector3 &lr)
{
	// order them from lowest to highest
	if (ul.x <= lr.x) {
		xpos[0] = ul.x;
		xpos[1] = lr.x;
	} else {
		xpos[1] = ul.x;
		xpos[0] = lr.x;
	}

	if (ul.y <= lr.y) {
		ypos[0] = ul.y;
		ypos[1] = lr.y;
	} else {
		ypos[1] = ul.y;
		ypos[0] = lr.y;
	}

	if (ul.z <= lr.z) {
		zpos[0] = ul.z;
		zpos[1] = lr.z;
	} else {
		zpos[1] = ul.z;
		zpos[0] = lr.z;
	}
}


colAABBf::colAABBf(float x1, float x2, float y1, float y2, float z1, float z2)
{
	// order them from lowest to highest
	if (x1 <= x2) {
		xpos[0] = x1;
		xpos[1] = x2;
	} else {
		xpos[1] = x1;
		xpos[0] = x2;
	}
	
	if (y1 <= y2) {
		ypos[0] = y1;
		ypos[1] = y2;
	} else {
		ypos[1] = y1;
		ypos[0] = y2;
	}

	if (z1 <= z2) {
		zpos[0] = z1;
		zpos[1] = z2;
	} else {
		zpos[1] = z1;
		zpos[0] = z2;
	}
}


//If your ray is r=A+tB where B is normalized and the sphere center is at C then the shortest dist is:
//distsquared = (|C-A|^2 - |(C-A).B|^2)



//bool TestPointBehindPlane(Vector3 &p, Plane3D &pl)
//{
//	if (pl.findDist(p) < 0) return true;
//	else return false;
//}


