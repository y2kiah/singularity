//	----==== TGA.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			2/04
//	Description:	The TGATexture class facilitates loading of a TGA file up to 32 bits
//					for an application using OpenGL 
//----------------------------------------------------------------------------------------

#include "tga.h"
#include "console.h"


/*-----------------
---- FUNCTIONS ----
-----------------*/

////////// class TGATexture //////////


bool TGATexture::getTGAData(const char *fileName, FILE *fTGA)
{
	TGAInfo	tga;	// TGA info

	if (fread(&tga.header, sizeof(tga.header), 1, fTGA) == 0) {										
		console.addLineErr("     could not read TGA info header: \"%s\"",fileName);
		fclose(fTGA);
		return false;
	}	

	width		= tga.header[1] * 256 + tga.header[0];			// Determine The TGA Width	(highbyte*256+lowbyte)
	height		= tga.header[3] * 256 + tga.header[2];			// Determine The TGA Height	(highbyte*256+lowbyte)
	bpp			= tga.header[4];								// Determine the bits per pixel
	tga.width	= width;										// Copy width into local structure						
	tga.height	= height;										// Copy height into local structure
	tga.bpp		= bpp;											// Copy BPP into local structure

	if ((width <= 0) || (height <= 0) || ((bpp != 24) && (bpp != 32))) {
		console.addLineErr("     invalid TGA texture information: \"%s\"",fileName);
		fclose(fTGA);
		return false;
	}

	tga.bytesPerPixel	= tga.bpp / 8;								// Compute the number of BYTES per pixel
	tga.imageSize		= tga.bytesPerPixel*tga.width*tga.height;	// Compute the total amout ofmemory needed to store data
	imageData			= new unsigned char[tga.imageSize];			// Allocate that much memory

	if (imageData == 0) {
		console.addLineErr("     could not allocate memory for TGA image: \"%s\"",fileName);
		fclose(fTGA);
		return false;
	}

	if (fread(imageData,1,tga.imageSize,fTGA) != tga.imageSize) {
		console.addLineErr("     could not read TGA image data: \"%s\"",fileName);
		delete [] imageData;
		fclose(fTGA);
		return false;
	}

	// Byte swapping
	for (int cswap = 0; cswap < tga.imageSize; cswap += tga.bytesPerPixel) {
		imageData[cswap] ^= imageData[cswap+2] ^= imageData[cswap] ^= imageData[cswap+2];
	}

	fclose(fTGA);
	return true;
}


bool TGATexture::loadTGA(const char *fileName)
{
	TGAHeader		tgaHeader;									// TGA header
	unsigned char	TGACompare[12] = {0,0,2,0,0,0,0,0,0,0,0,0};	// Uncompressed TGA header

	// Try to open the file
	FILE *fTGA;

	// check if file exists
	if (fopen_s(&fTGA,fileName,"rb") != 0) {		
		console.addLineErr("     file \"%s\" not found",fileName);
		return false;
	}

	// Try to read the TGA header
	if (fread(&tgaHeader, sizeof(TGAHeader), 1, fTGA) == 0) {
		console.addLineErr("     could not read TGA header: \"%s\"",fileName);
		if (fTGA != 0) fclose(fTGA);
		return false;
	}

	// Check if it's a TGA file
	if (memcmp(&TGACompare, &tgaHeader, sizeof(tgaHeader)) == 0) {
		return getTGAData(fileName, fTGA);	// get the file information

	} else {

		console.addLineErr("     unsupported TGA format: \"%s\"",fileName);
		fclose(fTGA);
		return false;
	}
}


TGATexture::TGATexture()
{
	imageData = 0;
	bpp = width = height = 0;
}