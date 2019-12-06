//	----==== ENGINE.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	This class owns all of the objects used for the engine
//	--------------------------------------------------------------------------------


#ifndef ENGINE_H
#define ENGINE_H

#include "options.h"
#include "UTILITYCODE\singleton.h"

/*---------------
---- DEFINES ----
---------------*/

#define engine				Engine::instance()	// used to access Engine instance globally

#define VIEWANGLE			60.0f
#define GRAVITY				-32.2f

/*------------------
---- STRUCTURES ----
------------------*/

class Console;
class Player;
class Level;
class GLFont;
class LookupManager;
class MouseManager;
class KeyboardManager;
class TextureManager;
class ShaderManager;
class CameraManager;
class MeshManager;
class ObjectManager;


struct KeyStates {
	bool	FORWARD;
	bool	BACKWARD;
	bool	STRAFELEFT;
	bool	STRAFERIGHT;
	bool	JUMP;
	bool	CROUCH;
	bool	RUNTOGGLE;
	bool	LEANLEFT;
	bool	LEANRIGHT;

	KeyStates() {
		FORWARD = BACKWARD = STRAFELEFT = STRAFERIGHT = JUMP =
		CROUCH = RUNTOGGLE = LEANLEFT = LEANRIGHT = false;
	}
};


class Engine : public Singleton<Engine> {
	private:

		GameOptions			*optionsInst;
		GlExtensionOptions	*extOptionsInst;
		Console				*consoleInst;
		MouseManager		*mouseInst;
		KeyboardManager		*keyboardInst;
		LookupManager		*lookupInst;		
		TextureManager		*textureInst;
		ShaderManager		*shaderInst;
		CameraManager		*cameraInst;
		MeshManager			*meshInst;
		ObjectManager		*objectMgrInst;

		Level				*levelInst;
		
		bool				toggleButton[256];

	public:

		KeyStates			keyState;

		ConsoleOptions		cOptions;
		
		Player				*player;
		GLFont				*font;


		bool			toggleButtonCheck(const short vk_code);

		inline void		resetKeyStates(void);
		void			getKeyStates(void);
		void			playGame(void);

		bool			checkExtension(const char *extName) const;
		void			initExtensions(void);
		void			initOpenGL(void);
		void			setUpGame(void);

		///// TEMP in place of console variables
		bool actuallyocclude;
		bool wireframe;
		int lod;		
		bool drawCached;
		bool drawDetailLayer;
		bool usePlayerCameraInDevMode;
		bool exitEngine;
		/////

		explicit Engine();
		~Engine();
};

#endif