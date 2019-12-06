//	----==== TGA.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	The TGATexture class facilitates loading of a TGA file up to 32 bits
//					for an application using OpenGL 
//----------------------------------------------------------------------------------------


#ifndef TGA_H
#define TGA_H

#include <cstdio>


/*------------------
---- STRUCTURES ----
------------------*/


struct TGAHeader {
	unsigned char	header[12];					// TGA file header
};


struct TGAInfo {
	unsigned char	header[6];					// First 6 wseful bytes from the header
	unsigned int	bytesPerPixel;				// Holds number of bytes per pixel used in the TGA file
	unsigned int	imageSize;					// Used to store the image size when setting aside RAM
	unsigned int	temp;						// Temporary variable
	unsigned int	height;						// Height of image
	unsigned int	width;						// Width of image
	unsigned int	bpp;						// Bits per pixel
};


class TGATexture {
	private:

		///// Attributes

		unsigned char	*imageData;				// Image data (up to 32 bits)
		unsigned int	bpp;					// Image color depth in bits per pixel
		unsigned int	width;					// Image width
		unsigned int	height;					// Image height

		bool			getTGAData(const char *fileName, FILE *fTGA);

	public:

		///// Accessors

		const unsigned char	*	getImageData() const		{ return imageData; }
		unsigned int			getBpp() const				{ return bpp; }
		unsigned int			getBytesPerPixel() const	{ return bpp / 8; }

		unsigned int	getWidth() const	{ return width; }
		unsigned int	getHeight() const	{ return height; }

		///// Other functions

		bool			loadTGA(const char *fileName);

		TGATexture();
		~TGATexture()	{ delete [] imageData; }
};


#endif