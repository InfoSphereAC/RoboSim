/*
 *  AndroidSpeaker.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 20.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "AndroidSpeaker.h"

#include "../../Robot.h"
#include "AndroidSoundBuffer.h"
#include "AndroidSoundController.h"
#include "../../Vec4.h"

RobotSpeaker::RobotSpeaker(Robot *aRobot, SoundController *aController) throw()
: robot(aRobot), controller(aController)
{
	//TODO: Implement sound on Android
	currentBuffer = NULL;
	deleteOldBufferWhenDone = false;
}

RobotSpeaker::~RobotSpeaker()
{
	//TODO: Implement sound on Android
	if (deleteOldBufferWhenDone && currentBuffer) delete currentBuffer;
}

void RobotSpeaker::update() throw()
{
	//TODO: Implement sound on Android
}

void RobotSpeaker::playSoundBuffer(SoundBuffer *buffer, bool loops, float volume, bool deleteWhenDone) throw()
{	
	//TODO: Implement sound on Android
	if (deleteOldBufferWhenDone && currentBuffer) delete currentBuffer;
	currentBuffer = buffer;
		
	deleteOldBufferWhenDone = deleteWhenDone;
}

void RobotSpeaker::playTone(unsigned frequency, unsigned duration, bool loops, float volume) throw()
{
	//TODO: Implement sound on Android
	SoundBuffer *newBuffer = new SoundBuffer(frequency, duration);
	playSoundBuffer(newBuffer, loops, volume, true);
}

void RobotSpeaker::playFile(const char *file, bool loops, float volume) throw()
{
	//TODO: Implement sound on Android
	SoundBuffer *buffer = controller->bufferWithFilename(file);
	playSoundBuffer(buffer, loops, volume, false);
}

float RobotSpeaker::noiseLevelAtPoint(const float4 &position) const throw()
{
	//TODO: Implement sound on Android
	return 0.0f;
}
