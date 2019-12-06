//	----==== TEXTUREMANAGER.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:	
//	--------------------------------------------------------------------------------


#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <vector>
#include "texture.h"
#include "UTILITYCODE\msgassert.h"
#include "UTILITYCODE\singleton.h"
#include "UTILITYCODE\timer.h"


/*---------------
---- DEFINES ----
---------------*/

#define texture		TextureManager::instance()	// used to access the TextureManager instance globally


/*------------------
---- STRUCTURES ----
------------------*/

class CHandler;
class TextureMgr_CAccess;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
class TextureManager : public Singleton<TextureManager> {
	friend class TextureMgr_CAccess;	

	private:

		///// Variables

		std::vector<Texture*>	textures;
		std::vector<_int64>		lastAccessTimes;

		float		budgetInMB;
		float		timeoutInSec;

		CHandler	*cHandler;

		///// Functions

		void	removeExpiredTextures(void);

	public:
		

		///// Functions

		int		push(Texture *t);
		int		loadFromFile(	const std::string &filename, GLenum clamp, GLenum mode,
								bool mipmap, bool bilinear, bool compress, bool keepData);

		int		createFromData(	const void *data, int internalFormat, int width, int height, GLenum type, GLenum clamp,
								GLenum mode, bool mipmap, bool bilinear, bool compress, const std::string &ident);

		void	clear(void);

		const Texture &getTexture(int i)	{	msgAssert(i >= 0 && i < textures.size(), "texture out of range");
												lastAccessTimes[i] = timer.getCurrentTime();
												return *textures[i];
											}

		// Accessors
//		const TextureOptions &getOptions(void) const { return tOptions; }

		///// Constructors / Destructor

		explicit TextureManager();
		~TextureManager();
};


#endif
