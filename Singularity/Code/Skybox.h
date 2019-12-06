/* ----==== SKYBOX.H ====---- */

#ifndef SKYBOX_H
#define SKYBOX_H

#include <string>
#include <vector>
#include "MATHCODE\vector4.h"

/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class Skybox_CAccess;


class SkyLayer {
	private:
		bool			active;
		float			cu, cv, uSpeed, vSpeed;
		unsigned int	textureID;

	public:
		bool	isActive()	const { return active; }
		float	getUSpeed()	const { return uSpeed; }
		float	getVSpeed()	const { return vSpeed; }
		int		getTexID()	const { return textureID; }

		void	update(void);
		void	render(void) const;

		void	setActive(bool set) { active = set; }
		void	setSpeed(float us, float vs);

		explicit	SkyLayer(int texID, float us, float vs);
};


class Skybox {
	friend class Skybox_CAccess;

	private:
		int						cubeMapTexID;	// sky box texture ID
		unsigned int			textureID[6];	// sky box texture IDs
		std::vector<SkyLayer*>	layer;			// cloud layers, up to 5

		CHandler				*cHandler;

		///// TEMP VARS
		float					sunAngle;
		Vector4					sunPos;

	public:		

		float					getSunAngle(void) const { return sunAngle; }
		const Vector4	&		getSunPos(void) const { return sunPos; }

		void					setSunAngle(float _a) { sunAngle = _a; }
		void					setSunPos(const Vector4 &_p) { sunPos.assign(_p); }

		void					render(void);
		bool					loadSkybox(const std::string &filename);

		explicit Skybox();
		~Skybox();
};

#endif
