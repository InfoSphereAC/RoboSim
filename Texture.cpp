/*
 *  Texture.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Texture.h"

#include <cstdarg>
#include <stdint.h>
#include <fstream>
#include <istream>

#include "ByteOrder.h"
#include "OpenGL.h"

#ifdef ANDROID_NDK
#include "android_ifstream.h"
#endif

static void throwRuntimeError(const char *filename, const char *format, ...)
{
	va_list args;
	va_start(args, format);
	
	char fullText[1024];
	vsnprintf(fullText, sizeof(fullText), format, args);
	
	throw std::runtime_error(fullText);
}

namespace
{
	// All of a TGA file header, for easier access
#ifdef _MSC_VER
#pragma pack (1)
#endif
	struct TGAHeader
	{
		uint8_t IDLength;
		uint8_t colorMapType;
		uint8_t imageType;
		uint16_t colorMapOffset;
		uint16_t colorMapLength;
		uint8_t colorMapEntrySize;
		uint16_t xorigin;
		uint16_t yorigin;
		uint16_t width;
		uint16_t height;
		uint8_t depth;
		uint8_t descriptor;
        
        void swapToHost()
        {
            colorMapOffset = SwapU16LittleToHost(colorMapOffset);
            colorMapLength = SwapU16LittleToHost(colorMapLength);
            xorigin = SwapU16LittleToHost(xorigin);
            yorigin = SwapU16LittleToHost(yorigin);
            width = SwapU16LittleToHost(width);
            height = SwapU16LittleToHost(height);
        }
#ifdef _MSC_VER
	};
#else
	} __attribute__((packed));
#endif
}

#ifdef ANDROID_NDK
Texture::Texture(AndroidAssetManager *manager, const char *filename) throw (std::runtime_error)
{
	android_ifstream file(manager, filename);
#else
Texture::Texture(const char *filename) throw (std::runtime_error)
{
	std::ifstream file(filename, std::ios::binary);
#endif
	if (!file) throwRuntimeError(filename, "Could not open texture image file");
	
	// Assert a couple of things about the file so we have less cases to deal with.
	TGAHeader header;
	file.read(reinterpret_cast<char *> (&header), sizeof(header));
    header.swapToHost();
	if (header.IDLength != 0) throwRuntimeError(filename, "Texture image format not supported (reason: Uses ID)");
	if (header.colorMapType != 0) throwRuntimeError(filename, "Texture image format not supported (reason: Uses color map)");
	if (header.imageType != 2) throwRuntimeError(filename, "Texture image format not supported (reason: Unsupported image type. Expected 2, got %d)", header.imageType);
	if (header.colorMapLength != 0) throwRuntimeError(filename, "Texture image format not supported (reason: Uses color map)");
	if (header.depth != 24 && header.depth != 32) throwRuntimeError(filename, "Texture image format not supported (reason: depth is %u, should be 24 or 32)", header.depth);
	
	bool hasAlpha = (header.depth == 32);
	GLenum internalFormat = hasAlpha ? GL_RGBA : GL_RGB;
	GLenum format = hasAlpha ? GL_RGBA : GL_RGB;
	
	char *data = new char [header.width * header.height * (hasAlpha ? 4 : 3)];
	file.read(data, header.width * header.height * (hasAlpha ? 4 : 3));
	
	// Swap BGRA to RGBA
	unsigned stride = hasAlpha ? 4 : 3;
	for (int i = 0; i < (header.width * header.height); i++)
	{
		char blue = data[i*stride + 0];
		char green = data[i*stride + 1];
		char red = data[i*stride + 2];
		data[i*stride + 0] = red;
		data[i*stride + 1] = green;
		data[i*stride + 2] = blue;
	}

	glGenTextures(1, &identifier);
	glBindTexture(GL_TEXTURE_2D, identifier);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, header.width, header.height, 0, format, GL_UNSIGNED_BYTE, data);
		
	delete [] data;
}

Texture::~Texture() throw()
{
	glDeleteTextures(1, &identifier);
}

void Texture::set() const throw()
{
	glBindTexture(GL_TEXTURE_2D, identifier);
}
