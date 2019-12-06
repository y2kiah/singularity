//	----==== SCREENMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			6/04
//	Description:	Organizes several display options into a manager
//	---------------------------------------------------------------------------


#ifndef SCREENMANAGER_H
#define SCREENMANAGER_H

#include "singleton.h"


/*---------------
---- DEFINES ----
---------------*/

#define sManager	ScreenManager::instance()
#define screen		ScreenManager::instance()


/*------------------
---- STRUCTURES ----
------------------*/

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class ScreenManager : public Singleton<ScreenManager> {

	private:

		int		RESX;
		int		RESY;
		int		REFRESH;
		int		COLORDEPTH;
		int		ZDEPTH;
		bool	NICEGRAPHICS;
		bool	FULLSCREEN;
//		float	VIEWANGLE;
//		float	VIEWDIST;

	public:
		
		int		getResX() const { return RESX; }
		int		getResY() const { return RESY; }
		int		getRefresh() const { return REFRESH; }
		int		getColorDepth() const { return COLORDEPTH; }
		int		getZDepth() const { return ZDEPTH; }
		bool	getFullScreen() const { return FULLSCREEN; }
		bool	getNiceGraphics() const { return NICEGRAPHICS; }

//		void	initOpenGL(void);

		explicit ScreenManager(int resX, int resY, int refresh, int colorDepth, int zDepth, 
							   bool niceGraphics, bool fullScreen);

		explicit ScreenManager();
		~ScreenManager() {}
};

#endif