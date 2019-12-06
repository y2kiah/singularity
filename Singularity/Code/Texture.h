//	----==== TEXTURE.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	The Texture class is designed to provide an easy means of loading many
//					different texture types (currently BMP and TGA) from disk, or creating
//					them from local data, and having them all represented with one common
//					data type. The image data is given directly to OpenGL with the option
//					to save the image data in RAM for future reading.
//	Dependencies:	This class requires the TGATexture class for loading of TGA textures
//----------------------------------------------------------------------------------------



#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL\glew.h>
#include <string>
#include "UTILITYCODE\color.h"

/*------------------
---- STRUCTURES ----
------------------*/


class Texture {
	private:

		///// Attributes
		
		int				w, h;				// width, height
		int				bytesPerPixel;		// bytes per pixel
		std::string		filename;			// filename of the texture
		unsigned char	*imageData;			// Stored image data
		unsigned int	glTexID;			// OpenGL texture location

		///// Loaders
		
		bool	loadTextureTGA(	const std::string &_filename, GLenum _clamp, GLenum _mode,
								bool _mipmap, bool _bilinear, bool _compress, bool _keepData);

/*		bool	loadTextureBMP(	const std::string &_filename, GLenum _clamp, GLenum _mode,
								bool _mipmap, bool _bilinear, bool _compress);
*/
	public:
		
		///// Accessors

		bool					hasImageData(void) const	{ return (imageData != 0); }
		bool					getIsAlpha(void) const		{ return bytesPerPixel == 4; }
		int						getWidth(void) const		{ return w; }
		int						getHeight(void) const		{ return h; }
		int						getBytesPP(void) const		{ return bytesPerPixel; }
		const std::string &		getFilename(void) const		{ return filename; }
		unsigned int			getGLTexID(void) const		{ return glTexID; }

		Color3f					findAvgColor(const Color3f &c, int width, int height, int startX, int startY);

		///// Mutators
		
		void			clearImageData(void) { delete [] imageData; imageData = 0; }

		bool			loadTexture(const std::string &_filename, GLenum _clamp, GLenum _mode,
							bool _mipmap, bool _bilinear, bool _compress, bool _keepData);
	
		bool			createTexture(	const void *_data, int _internalFormat, int _width, int _height, GLenum _type, GLenum _clamp,
							GLenum _mode, bool _mipmap, bool _bilinear, bool _compress, const std::string &ident);
		
//		void			createAlphaTexture(const float *data, int width, int height, GLenum clamp, GLenum mode);


		float			getObjectSizeInKB() const { return (sizeof(Texture) + (hasImageData() ? bytesPerPixel*w*h : 0)) / 1024.0f; }
		std::string		toString(void) const;

		///// Constructor / Destructor
		explicit	Texture();
		~Texture();
};


#endif