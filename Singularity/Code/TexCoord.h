/* ----==== TEXCOORD.H ====---- */

#ifndef TEXCOORD_H
#define TEXCOORD_H

/*------------------
---- STRUCTURES ----
------------------*/

struct TexCoord {
	float u, v;

	bool operator ==(TexCoord &_t) const { if(u == _t.u && v == _t.v) return true; else return false; }
};

#endif