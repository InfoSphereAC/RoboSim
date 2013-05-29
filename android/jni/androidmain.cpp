/*
 *  androidmain.c
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 19.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <android/log.h>
#include "asset_manager.h"
#include <jni.h>
#include <map>

#include "AndroidFileChooser.h"

#include "../../Controller.h"

/*
#include <android/log.h>
__android_log_print(ANDROID_LOG_ERROR,"Robosim","At bottom");
*/


#define JNI_NAME(name) JNICALL Java_de_ferroequinologist_robosimandroid_Controller_ ## name

/*
 * Note: To store a C++ controller object in a Java Controller object, one could
 * store the pointer in a Java long, but due to 32/64-bittiness and Java not
 * knowing a signed type, I'd prefer avoiding that.
 */
std::map<jint, Controller *>controllers;
static AndroidFileChooser *chooser;

static Controller *ControllerForIndex(jint index)
{
	std::map<jint, Controller *>::iterator found = controllers.find(index);
	if (found != controllers.end()) return found->second;
	else return NULL;
}

extern "C" jint JNI_NAME(load)(JNIEnv *env, jclass obj, jobject javaController, jint mode, jint flags, jstring path, jstring address, jstring port, jstring assetPath)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Creating controller with mode %d, path %p, address %p, port %p in environment %p", mode, path, address, port, env);
	const char *cName = path ? env->GetStringUTFChars(path, NULL) : NULL;
	const char *cAddress = address ? env->GetStringUTFChars(address, NULL) : NULL;
	const char *cPort = port ? env->GetStringUTFChars(path, NULL) : NULL;
	const char *cAssetPath = env->GetStringUTFChars(assetPath, NULL);
	
	AndroidAssetManager *manager = new AndroidAssetManager(cAssetPath);
	chooser = new AndroidFileChooser(env, javaController);
	
	Controller *controller = new Controller(manager, Controller::NetworkMode(mode), flags, cName, cAddress, cPort);
	
	if (cName) env->ReleaseStringUTFChars(path, cName);
	if (cAddress) env->ReleaseStringUTFChars(address, cAddress);
	if (cPort) env->ReleaseStringUTFChars(port, cPort);
	env->ReleaseStringUTFChars(assetPath, cAssetPath);
	
	jint unusedIndex = controllers.size();
	while (controllers.find(unusedIndex) != controllers.end()) unusedIndex++;
	controllers[unusedIndex] = controller;
	return unusedIndex;
}

extern "C" void JNI_NAME(destroy)(JNIEnv *env, jclass clazz, jint index)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;
	
	controllers.erase(controllers.find(index));
	delete controller;
}

extern "C" void JNI_NAME(shutDown)(JNIEnv *env, jclass clazz, jint index)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->shutDown();
}

extern "C" void JNI_NAME(keyUp)(JNIEnv *env, jclass clazz, jint index, jint character, jboolean isPrintable)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->keyUp(character, isPrintable);
}

extern "C" void JNI_NAME(keyDown)(JNIEnv *env, jclass clazz, jint index, jint character, jboolean isPrintable)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;
	
	controller->keyDown(character, isPrintable);
}

extern "C" void JNI_NAME(initVideo)(JNIEnv *env, jclass clazz, jint index)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	try
	{
		controller->initVideo();
	}
	catch (std::exception &e)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Fatal error in initVideo: %s", e.what());
	}
}

extern "C" void JNI_NAME(initAudio)(JNIEnv *env, jclass clazz, jint index)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->initAudio();
}

extern "C" void JNI_NAME(draw)(JNIEnv *env, jclass clazz, jint index)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->draw();
}

extern "C" void JNI_NAME(resize)(JNIEnv* env, jclass clazz, jint index, jint width, jint height)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->setWindowSize(width, height);
}

extern "C" void JNI_NAME(update)(JNIEnv *env, jclass clazz, jint index, jfloat timeDiff)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;
	
	controller->update(timeDiff);
}

extern "C" void JNI_NAME(setScreenOrientation)(JNIEnv *env, jclass clazz, jint index, jint orientation)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->setScreenOrientation(Controller::ScreenOrientation(orientation));
}

extern "C" void JNI_NAME(scroll)(JNIEnv *env, jclass clazz, jint index, jint amount)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;
	
	controller->scroll(amount);
}

extern "C" void JNI_NAME(mouseDown)(JNIEnv *env, jclass clazz, jint index, jfloat positionX, jfloat positionY, jint button)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->mouseDown(positionX, positionY, button);
}

extern "C" void JNI_NAME(mouseUp)(JNIEnv *env, jclass clazz, jint index, jfloat positionX, jfloat positionY, jint button)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	try
	{
		controller->mouseUp(positionX, positionY, button);
	}
	catch(std::exception &e)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Fatal error in mouseUp: %s", e.what());
	}
}

extern "C" void JNI_NAME(touchDown)(JNIEnv *env, jclass clazz, jint controllerIndex, jint touchIndex, jfloat positionX, jfloat positionY)
{
	Controller *controller = ControllerForIndex(controllerIndex);
	if (!controller) return;

	try
	{
		controller->touchDown(touchIndex, positionX, positionY);
	}
	catch(std::exception &e)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Fatal error in touchUp: %s", e.what());
	}
}

extern "C" void JNI_NAME(touchMoved)(JNIEnv *env, jclass clazz, jint controllerIndex, jint touchIndex, jfloat positionX, jfloat positionY)
{
	Controller *controller = ControllerForIndex(controllerIndex);
	if (!controller) return;
	
	controller->touchMoved(touchIndex, positionX, positionY);
}

extern "C" void JNI_NAME(touchUp)(JNIEnv *env, jclass clazz, jint controllerIndex, jint touchIndex)
{
	Controller *controller = ControllerForIndex(controllerIndex);
	if (!controller) return;
	
	try
	{
		controller->touchUp(touchIndex);
	}
	catch(std::exception &e)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Fatal error in touchUp: %s", e.what());
	}
}

extern "C" void JNI_NAME(touchCancelled)(JNIEnv *env, jclass clazz, jint controllerIndex, jint touchIndex)
{
	Controller *controller = ControllerForIndex(controllerIndex);
	if (!controller) return;
	
	controller->touchCancelled(touchIndex);
}

extern "C" void JNI_NAME(rotateCamera)(JNIEnv *env, jclass clazz, jint index, jfloat radians)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->rotateCamera(radians);
}

extern "C" void JNI_NAME(moveCamera)(JNIEnv *env, jclass clazz, jint index, jfloat x, jfloat y)
{
	Controller *controller = ControllerForIndex(index);
	if (!controller) return;

	controller->moveCamera(x, y);
}

extern "C" void JNI_NAME(fileChosen)(JNIEnv *env, jclass clazz, jstring filename)
{
	const char *cName = env->GetStringUTFChars(filename, NULL);
	
	chooser->fileChosen(cName);
	
	env->ReleaseStringUTFChars(filename, cName);

}
