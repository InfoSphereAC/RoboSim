/*
 *  android_ifstream.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "android_ifstream.h"
#include <android/log.h>

#define MAX_READ_SIZE 100000

android_streambuf::android_streambuf()
: manager(0), length(0), data(0), returnedData(false), hasToDeleteData(false)
{
}

bool android_streambuf::open(AndroidAssetManager *assetManager, const char *cFilename)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Opening stream to %s have manager <%p>", cFilename, assetManager);
	
	char filenameInArchive[1024];
	snprintf(filenameInArchive, sizeof(filenameInArchive), "assets/%s.mp3", cFilename);
	
	// Try without .mp3 if not found
	if (!assetManager->fileExists(filenameInArchive))
		snprintf(filenameInArchive, sizeof(filenameInArchive), "assets/%s", cFilename);
	
	// Still not? Then there's no such file.
	if (!assetManager->fileExists(filenameInArchive))
		return false;
	
	length = assetManager->getOriginalFileSize(filenameInArchive);
	
	if (assetManager->getFileIsCompressed(filenameInArchive))
	{
		data = (const char *) assetManager->getDecompressedCopy(filenameInArchive);
		hasToDeleteData = true;
	}
	else
	{
		data = (const char *) assetManager->getRawFilePointer(filenameInArchive);
		hasToDeleteData = false;
	}

	returnedData = false;
		
	return true;
}

android_streambuf::~android_streambuf()
{
	if (hasToDeleteData)
		delete [] data;
}

android_streambuf::int_type android_streambuf::underflow()
{
	if (!returnedData)
	{
		size_t oldReadPointer = gptr() - eback();
						
		returnedData = true;
		
		setg((char_type *) data, (char_type *) &data[oldReadPointer], (char_type *) &data[length]);
		return int_type(data[oldReadPointer]);
	}
	else
		return traits_type::eof();
}

android_ifstream::android_ifstream(AndroidAssetManager *manager, const char *filename)
{
	this->init(&streambuf);
	if (!streambuf.open(manager, filename))
		this->setstate(std::ios::failbit);
	else
		this->clear();
}
