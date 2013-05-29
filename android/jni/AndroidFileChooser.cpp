//
//  AndroidFileChooser.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 12.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "AndroidFileChooser.h"

#include <android/log.h>

AndroidFileChooser::AndroidFileChooser(JNIEnv *anEnv, jobject aController)
{
	if (!anEnv || !aController)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Invalid arguments to creation of file chooser.");
	}
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Creating file chooser. Env %p, aController %p", anEnv, aController);
	env = anEnv;
	javaController = env->NewGlobalRef(aController);
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Got global ref.");
	method = env->GetMethodID(env->GetObjectClass(javaController), "startFileChooser", "()V");
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "File chooser created");
	
	FileChooser::setSharedIfNoneExists(this);
}

AndroidFileChooser::~AndroidFileChooser()
{
	env->DeleteGlobalRef(javaController);
}

void AndroidFileChooser::run(FileChooserDelegate *aDelegate)
{
	delegate = aDelegate;
	
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Running file chooser");
	env->CallVoidMethod(javaController, method);
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "File chooser started");
}

void AndroidFileChooser::fileChosen(const char *filename)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "File chooser found result \"%s\"", filename);
	if (!delegate) return;
	
	delegate->fileChooserFoundFile(filename);
}