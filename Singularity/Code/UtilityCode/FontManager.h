//	----==== FONTMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	FontManager is responsible for maintaining all of the fonts
//					that the engine will use, and is also responsible for
//					displaying those fonts on the screen.
//	----------------------------------------------------------------------------


#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "singleton.h"
#include "glfont.h"


/*---------------
---- DEFINES ----
---------------*/

#define fManager	FontManager::instance()		// used to access the FontManager instance globally
#define font		FontManager::instance()


/*------------------
---- STRUCTURES ----
------------------*/

class FontManager : public Singleton<FontManager> {
	private:

		///// Variables

		std::vector<GLFont*>	fonts;

	public:

		///// Functions

		const GLFont &getFont(int i) const { return *fonts.getData(i); }

		// Constructors / Destructor

		explicit FontManager();
		~FontManager() {}
};


#endif