//
//  FileChooser.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 12.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "FileChooser.h"

#if defined(ANDROID_NDK)
#include "android/jni/AndroidFileChooser.h"
#elif defined(IPHONE) || defined(IPHONE_SIMULATOR)
#include "iOSFileChooser.h"
#elif defined(__APPLE_CC__)
#include "MacOSXFileChooser.h"
#elif defined(_WIN32)
#include "Windows/WindowsFileChooser.h"
#else
#error No implementation of this for this platform
#endif

namespace
{
	FileChooser *sharedChooser = 0;
}

void FileChooser::setSharedIfNoneExists(FileChooser *chooser)
{
	if (!sharedChooser)
		sharedChooser = chooser;
}

FileChooser *FileChooser::sharedFileChooser()
{
	if (!sharedChooser)
	{
#if defined(ANDROID_NDK)
		sharedChooser = new AndroidFileChooser(0, 0);
#elif defined(IPHONE) || defined(IPHONE_SIMULATOR)
		sharedChooser = new iOSFileChooser();
#elif defined(__APPLE_CC__)
		sharedChooser = new MacOSXFileChooser();
#elif defined(_WIN32)
		sharedChooser = new WindowsFileChooser();
#else
#error No implementation of this for this platform
#endif
	}
	return sharedChooser;
}