/*
 *  android_ifstream.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <streambuf>
#include <istream>

#include "asset_manager.h"

class android_streambuf : public std::streambuf
{
	AndroidAssetManager *manager;
	long length;
	const char *data;
	bool returnedData;
	bool hasToDeleteData;
public:
	android_streambuf();
	bool open(AndroidAssetManager *manager, const char *filename);
	virtual ~android_streambuf();
protected:
	// Functions that have to be overridden
	virtual int_type underflow();
};

class android_ifstream : public std::istream
{
	android_streambuf streambuf;
public:
	android_ifstream(AndroidAssetManager *manager, const char *filename);
};
