//
//  AndroidFileChooser.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 12.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "../../FileChooser.h"

#include <jni.h>

class AndroidFileChooser : public FileChooser
{
	JNIEnv *env;
	jobject javaController;
	jmethodID method;
public:
	// Do not actually call that. Just public so that androidmain.cpp can call it.
	AndroidFileChooser(JNIEnv *env, jobject javaController);
	void fileChosen(const char *filename);
	~AndroidFileChooser();
	
	virtual void run(FileChooserDelegate *delegate);
};
