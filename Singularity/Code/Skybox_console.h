/* ----==== SKYBOX_CONSOLE.H ====---- */

#ifndef SKYBOX_CONSOLE_H
#define SKYBOX_CONSOLE_H

#include "console.h"

/*------------------
---- STRUCTURES ----
------------------*/

class Skybox_CAccess : public CHandler {
	private:
		void layerSpeed(int _layer, float _us, float _vs) const;
		void layerActivate(int _layer, float _us, float _vs) const;
		void layerDeactivate(int _layer) const;
		void layerActiveList(void) const;

	public:
		void handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const;

		explicit Skybox_CAccess();
		~Skybox_CAccess();
};

#endif
