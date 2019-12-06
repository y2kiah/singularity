//	----==== TEXTURE.CPP ====----
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


#include <sstream>
#include <algorithm>
#include "texture.h"
#include "tga.h"
#include "console.h"
#include "options.h"

/*-----------------
---- FUNCTIONS ----
-----------------*/


////////// class Texture //////////

//----------------------------------------------------------------------------------------
//	This function takes a specified rectangular chunk from the texture and returns the
//	average color of all pixels contained within that chunk. Out of bounds errors or lack
//	of image data in memory will cause a false return and c will be unaltered
//----------------------------------------------------------------------------------------
Color3f Texture::findAvgColor(const Color3f &c, int width, int height, int startX, int startY)
{
	if (!hasImageData()) return c;

	int totalR = 0, totalG = 0, totalB = 0, totalCount = 0;

	for (int y = startY; y < startY + height; y++) {
		for (int x = startX; x < startX + width; x++) {
			int index = bytesPerPixel * (y * w + x);
			totalR += imageData[index];
			totalG += imageData[index+1];
			totalB += imageData[index+2];
			totalCount++;
		}
	}

	if (totalCount == 0) return c;
	float invTotal = 1.0f / (float)totalCount;

	return (Color3f((float)totalR * invTotal, (float)totalG * invTotal, (float)totalB * invTotal));
}


//----------------------------------------------------------------------------------------
//	loadTexture automatically determines which type of texture you are trying to load by
//	searching for the file extension. It should be called to load all textures from disk
//----------------------------------------------------------------------------------------
bool Texture::loadTexture(const std::string &_filename, GLenum _clamp, GLenum _mode,
						  bool _mipmap, bool _bilinear, bool _compress, bool _keepData)
{
	if (_filename.length() == 0) {
		console.addLineErr("     texture loading failed, filename zero length");
		return false;
	}

	bool returnVal = false;
	
	// convert extension to lower case for comparison
	std::string extension = _filename.substr(_filename.length()-4,4);
	std::transform(extension.begin(), extension.end(), extension.begin(), tolower);

	if (extension == ".tga") {
		returnVal = loadTextureTGA(_filename, _clamp, _mode, _mipmap, _bilinear, _compress, _keepData);

	}/* else if (_strnicmp(".bmp",extension,4) == 0) {
		returnVal = loadTextureBMP(_fileName, _clamp, _mode, _mipmap, _bilinear);

	}*/ else {
		console.addLineErr("     texture loading failed, extension not recognized in \"%s\"", _filename.c_str());
		return false;
	}

	if (returnVal) {		
		console.addLine("     texture loaded: %s", toString().c_str());
	}

	return returnVal;
}


//----------------------------------------------------------------------------------------
//	This private function is called by the loadTexture function
//----------------------------------------------------------------------------------------
bool Texture::loadTextureTGA(const std::string &_filename, GLenum _clamp, GLenum _mode,
							 bool _mipmap, bool _bilinear, bool _compress, bool _keepData)
{
	glGenTextures(1, &glTexID);

	TGATexture tempTGA;
	if (!tempTGA.loadTGA(_filename.c_str())) {
		console.addLineErr("     error loading TGA texture: \"%s\": TGA did not load", _filename.c_str());
		return false;
	}

	filename = _filename;
	w = tempTGA.getWidth();
	h = tempTGA.getHeight();
	bytesPerPixel = tempTGA.getBpp() / 8;
	if (bytesPerPixel != 1 && bytesPerPixel != 3 && bytesPerPixel != 4) {
		console.addLineErr("     error loading TGA texture: \"%s\": unsupported bpp", _filename.c_str());
		return false;
	}

	glBindTexture(GL_TEXTURE_2D, glTexID);

	// get internal format
	GLint internalFmt;
	GLenum format;
	if (bytesPerPixel == 4) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = extOptions.USE_GL_EXT_texture_compression_s3tc ?
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_ARB;
		} else {
			internalFmt = GL_RGBA8;
		}
		format = GL_RGBA;
	} else if (bytesPerPixel == 3) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = extOptions.USE_GL_EXT_texture_compression_s3tc ?
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_ARB;
		} else {
			internalFmt = GL_RGB8;
		}
		format = GL_RGB;
	} else if (bytesPerPixel == 1) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = (extOptions.USE_GL_EXT_texture_compression_s3tc) ?
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_ARB;
		} else {
			internalFmt = GL_LUMINANCE8;
		}
		format = GL_LUMINANCE;
	}

	if (_bilinear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (!_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, tempTGA.getWidth(), tempTGA.getHeight(), 0, format, GL_UNSIGNED_BYTE, tempTGA.getImageData());

		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D, internalFmt, tempTGA.getWidth(), tempTGA.getHeight(), format, GL_UNSIGNED_BYTE, tempTGA.getImageData());

		}

	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (!_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, tempTGA.getWidth(), tempTGA.getHeight(), 0, format, GL_UNSIGNED_BYTE, tempTGA.getImageData());

		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D, internalFmt, tempTGA.getWidth(), tempTGA.getHeight(), format, GL_UNSIGNED_BYTE, tempTGA.getImageData());

		}
	}

	// Check if driver compressed the texture effectively
	if (_compress && extOptions.USE_GL_ARB_texture_compression) {
		GLint compression;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compression);
		if (compression == 0) console.addLineErr("     texture \"%s\" was not successfully compressed by the driver", _filename.c_str());
	}

	// Keep image data if specified
	if (_keepData) {
		int texSize = bytesPerPixel * w * h;
		imageData = new unsigned char[texSize];
		if (imageData) memcpy(imageData,tempTGA.getImageData(),texSize);
	} else {
		imageData = 0;
	}

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,_mode);

	return true;
}


//----------------------------------------------------------------------------------------
//	This private function is called by the loadTexture function
//----------------------------------------------------------------------------------------
/*bool Texture::loadTextureBMP(const char *_fileName, GLenum _clamp, GLenum _mode, bool _mipmap)
{
	numFrames = 1;
	texture = new unsigned int[numFrames];
	glGenTextures(numFrames,texture);

	AUX_RGBImageRec *tempBMP = auxDIBImageLoad(_fileName);
	if (!tempBMP) {
		console.addLine(255,64,64,"     error loading BMP texture: \"%s\"",_fileName);
		return false;
	}
	
	w = tempBMP->sizeX;
	h = tempBMP->sizeY;
	bytesPerPixel = 3;
	isAlpha = false;

	glBindTexture(GL_TEXTURE_2D,texture[0]); // change 0 to current frame that is being loaded

	if (gOptions.FILTERING) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (!gOptions.MIPMAP || !_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D,0,3,tempBMP->sizeX,tempBMP->sizeY,0,GL_RGB8,GL_UNSIGNED_BYTE,tempBMP->data);

		} else {
			if (gOptions.NICEGRAPHICS) glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
			else glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D,3,tempBMP->sizeX,tempBMP->sizeY,GL_RGB8,GL_UNSIGNED_BYTE,tempBMP->data);

		}

	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (!gOptions.MIPMAP || !_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D,0,3,tempBMP->sizeX,tempBMP->sizeY,0,GL_RGB8,GL_UNSIGNED_BYTE,tempBMP->data);

		} else {
			if (gOptions.NICEGRAPHICS) glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
			else glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D,3,tempBMP->sizeX,tempBMP->sizeY,GL_RGB8,GL_UNSIGNED_BYTE,tempBMP->data);

		}
	}

  	// Keep image data if specified
	if (keepImageData) {
		int texSize = bytesPerPixel * w * h;
		imageData = new unsigned char[texSize];
		if (imageData) memcpy(imageData,tempBMP->data,texSize);
	}

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,_mode);

	fileName = _fileName;

	return true;
}*/


bool Texture::createTexture(const void *_data, int _internalFormat, int _width, int _height, GLenum _type, GLenum _clamp,
							GLenum _mode, bool _mipmap, bool _bilinear, bool _compress, const std::string &ident)
{
	if (_internalFormat != 1 && _internalFormat != 3 && _internalFormat != 4) {
		console.addLineErr("     error creating texture: \"%s\": unsupported bpp", ident.c_str());
		return false;
	}

	glGenTextures(1, &glTexID);

	filename = ident;
	w = _width;
	h = _height;
	bytesPerPixel = _internalFormat;

	glBindTexture(GL_TEXTURE_2D, glTexID);

	// get internal format
	GLint internalFmt;
	GLenum format;
	if (bytesPerPixel == 4) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = (extOptions.USE_GL_EXT_texture_compression_s3tc) ?
				GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_ARB;			
		} else {
			internalFmt = GL_RGBA8;
		}
		format = GL_RGBA;
	} else if (bytesPerPixel == 3) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = (extOptions.USE_GL_EXT_texture_compression_s3tc) ?
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_ARB;
		} else {
			internalFmt = GL_RGB8;
		}
		format = GL_RGB;
	} else if (bytesPerPixel == 1) {
		if (_compress && extOptions.USE_GL_ARB_texture_compression) {
			internalFmt = (extOptions.USE_GL_EXT_texture_compression_s3tc) ?
				GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_ARB;
		} else {
			internalFmt = GL_LUMINANCE8;
		}
		format = GL_LUMINANCE;
	}

	if (_bilinear) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (!_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0, format, _type, _data);

		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D, internalFmt, w, h, format, _type, _data);

		}

	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		if (!_mipmap) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFmt, w, h, 0, format, _type, _data);

		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

			gluBuild2DMipmaps(GL_TEXTURE_2D, internalFmt, w, h, format, _type, _data);

		}
	}

	// Check if driver compressed the texture effectively
	if (_compress && extOptions.USE_GL_ARB_texture_compression) {
		GLint compression;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compression);
		if (compression == 0) console.addLineErr("     internal texture not compressed successfully");
	}

	imageData = 0;

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,_mode);

	return true;
}


//----------------------------------------------------------------------------------------
//	This creates an OpenGL alpha texture with float data. It is used primarily for texture
//	splatting within the program
//----------------------------------------------------------------------------------------
/*void Texture::createAlphaTexture(const float *_data, int _width, int _height, GLenum _clamp, GLenum _mode)
{
	glGenTextures(1, &glTexID);

	w = _width;
	h = _height;
	bytesPerPixel = 1;
	isAlpha = true;
	keepImageData = false;	// this only applies to images created with loadTexture

	glBindTexture(GL_TEXTURE_2D, glTexID); // change 0 to current frame that is being loaded

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _clamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _clamp);

	glTexImage2D(GL_TEXTURE_2D, 0, 2, _width, _height, 0, GL_LUMINANCE_ALPHA,GL_FLOAT, _data);

	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,_mode);
}*/


std::string Texture::toString(void) const
{
	std::ostringstream returnStr;
	returnStr << "\"" << filename << "\": " << w << "x" << h << ": " << bytesPerPixel*8 << "bpp" << 
		(hasImageData() ? ": DATA" : "");

	return returnStr.str();
}


Texture::Texture()
{
	imageData = 0;
	filename = "";
	w = h = bytesPerPixel = 0;
	glTexID = -1;
}


Texture::~Texture()
{
	clearImageData();
	glDeleteTextures(1, &glTexID);
}