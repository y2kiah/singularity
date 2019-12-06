/* ----==== TEXTUREMGR_CONSOLE.CPP ====---- */

#include "texturemgr_console.h"
#include "texturemanager.h"
#include "texture.h"
#include "options.h"
#include "engine.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class TextureList_CAccess //////////

void TextureMgr_CAccess::textureList(void) const
{
	if (texture.textures.empty()) {
		console.addLine("     list empty");
	} else {
		float sumSize = 0;
		for (int c = 0; c < texture.textures.size(); ++c) {
			float texSizeKB = texture.textures[c]->getObjectSizeInKB();
			sumSize += texSizeKB;

			console.addLine("     ID %i: %s: age %0.3fs: mem %0.3fK", c, texture.textures[c]->toString().c_str(),
				timer.calcNumSecondsSinceTime(texture.lastAccessTimes[c]), texSizeKB);
		}
		
		console.addLine("     Total: %i textures: %0.3fMB", texture.textures.size(), sumSize / 1024.0f);
	}
}


void TextureMgr_CAccess::handleCommands(const char *_callname, const std::vector<CVar*> &_varlist) const
{
	if (_stricmp(_callname,"texturelist")==0) textureList();
}


TextureMgr_CAccess::TextureMgr_CAccess()
{
	console.registerCommand("texturelist","()",this,0);
}


TextureMgr_CAccess::~TextureMgr_CAccess()
{
	console.unregisterCommand("texturelist");
}