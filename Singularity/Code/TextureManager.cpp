//	----==== TEXTUREMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			5/04
//	Description:
//	----------------------------------------------------------------------------


#include "texturemanager.h"
#include "texturemgr_console.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class TextureManager //////////

int TextureManager::push(Texture *t)
{
	// Check if it's in the list already, if so delete the new texture and return the current ID
	for (int c = 0; c < textures.size(); ++c) {
		if (_stricmp(t->getFilename().c_str(), textures[c]->getFilename().c_str()) == 0) {
			console.addLineErr("     texture \"%s\" already loaded, using existing ID of %i", textures[c]->getFilename().c_str(), c);
			delete t;
			return c;
		}
	}

	textures.push_back(t);
	lastAccessTimes.push_back(timer.queryCurrentTime());

	return textures.size() - 1;
}


int	TextureManager::loadFromFile(const std::string &filename, GLenum clamp, GLenum mode, bool mipmap,
								 bool bilinear, bool compress, bool keepData)
{
	Texture *newtex = new Texture;

	if (!newtex->loadTexture(filename, clamp, mode, mipmap, bilinear, compress, keepData)) {
		delete newtex;
		return -1;
	}

	return push(newtex);
}


int TextureManager::createFromData(const void *data, int internalFormat, int width, int height, GLenum type, GLenum clamp,
								   GLenum mode, bool mipmap, bool bilinear, bool compress, const std::string &ident)
{
	Texture *newtex = new Texture;

	if (!newtex->createTexture(	data, internalFormat, width, height, type, clamp,
								mode, mipmap, bilinear, compress, ident)) {
		delete newtex;
		return -1;
	}

	return push(newtex);
}


void TextureManager::clear(void)
{
	for (int t = 0; t < textures.size(); ++t) delete textures[t];
	textures.clear();
	lastAccessTimes.clear();
}


TextureManager::TextureManager() : Singleton<TextureManager>(*this)
{
	cHandler = new TextureMgr_CAccess;

	textures.reserve(10);
	lastAccessTimes.reserve(10);
}


TextureManager::~TextureManager()
{
	clear();
	delete cHandler;
}