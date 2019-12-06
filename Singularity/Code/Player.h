/* ----==== PLAYER.H ====---- */

#ifndef PLAYER_H
#define PLAYER_H

#include "MATHCODE\vector3.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Camera;
class CHandler;
class Player_CAccess;


class Player {
	friend class Player_CAccess;

	private:
		Vector3			vel, targetvel, velsave;
		float			targety;
		float			runspeed, vertspeed;
		float			maxspeed;
		float			height;
		bool			jump;
		char			name[20];

		bool			jumpkey, crouchkey, runtoggle;
		bool			drawHUD;

		int				viewCamID;

		CHandler		*cHandler;

	public:

		Vector3			p, viewp, newp;
		int				yangle, xangle, zangle, moveyangle, targetRollAngle;		

		void	input(void);
		void	update(void);
		void	renderHUD(void) const;
		void	switchHUD(void) { drawHUD = !drawHUD; }

		int		getViewCamID() const { return viewCamID; }
		
		explicit Player(const char *_n,float _x,float _y,float _z,int _xa,int _ya,int _za,float maxspeed);
		~Player();
};

#endif
