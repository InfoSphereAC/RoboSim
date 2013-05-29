/*
 *  RobotSpeaker.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "RobotSpeaker.h"

#ifdef __APPLE_CC__
#include <OpenAL/al.h>
#else
#include <al.h>
#endif

#include "Robot.h"
#include "SoundBuffer.h"
#include "SoundController.h"
#include "Vec4.h"

RobotSpeaker::RobotSpeaker(Robot *aRobot, SoundController *aController) throw()
: robot(aRobot), controller(aController)
{
	currentBuffer = NULL;
	deleteOldBufferWhenDone = false;
	
	alGenSources(1, &sourceID);
}

RobotSpeaker::~RobotSpeaker()
{
	alSourceStop(sourceID);
	alSourcei(sourceID, AL_BUFFER, 0);
	if (deleteOldBufferWhenDone && currentBuffer) delete currentBuffer;
	alDeleteSources(1, &sourceID);
}

void RobotSpeaker::update() throw()
{
	alSourcefv(sourceID, AL_POSITION, robot->getPosition().w.c_ptr());
	
	float4 velocity = robot->getPosition().x * (robot->getLeftTrackSpeed() + robot->getRightTrackSpeed()) * 0.5f;
	alSourcefv(sourceID, AL_VELOCITY, velocity.c_ptr());
}

void RobotSpeaker::playSoundBuffer(SoundBuffer *buffer, bool loops, float volume, bool deleteWhenDone) throw()
{
	alSourceStop(sourceID);
	if (buffer) alSourcei(sourceID, AL_BUFFER, buffer->getBufferID());
	else alSourcei(sourceID, AL_BUFFER, 0);
	
	if (deleteOldBufferWhenDone && currentBuffer) delete currentBuffer;
	currentBuffer = buffer;
	
	alSourcei(sourceID, AL_LOOPING, loops);
	alSourcef(sourceID, AL_GAIN, volume);
	alSourcePlay(sourceID);
	
	deleteOldBufferWhenDone = deleteWhenDone;
}

void RobotSpeaker::playTone(unsigned frequency, unsigned duration, bool loops, float volume) throw()
{
	SoundBuffer *newBuffer = new SoundBuffer(frequency, duration);
	playSoundBuffer(newBuffer, loops, volume, true);
}

void RobotSpeaker::playFile(const char *file, bool loops, float volume) throw()
{
	SoundBuffer *buffer = controller->bufferWithFilename(file);
	playSoundBuffer(buffer, loops, volume, false);
}

float RobotSpeaker::noiseLevelAtPoint(const float4 &position) const throw()
{
	// Check whether it is playing.
	ALint sourceState = 0;
	alGetSourcei(sourceID, AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PLAYING) return 0.0f;
	
	// Assuming default values:
	// attentuation model is AL_INVERSE_DISTANCE_CLAMPED
	float4 diff = robot->getPosition().w - position;
	float distance = diff.length();
	
	float referenceDistance = 0.0f;
	alGetSourcef(sourceID, AL_REFERENCE_DISTANCE, &referenceDistance);
	float maxDistance = 0.0f;
	alGetSourcef(sourceID, AL_MAX_DISTANCE, &maxDistance);
	float rolloffFactor = 0.0f;
	alGetSourcef(sourceID, AL_ROLLOFF_FACTOR, &rolloffFactor);
	
	distance = fminf(distance, referenceDistance);
	distance = fmaxf(distance, maxDistance);
	
	float definedVolume;
	alGetSourcef(sourceID, AL_GAIN, &definedVolume);
	float gain = referenceDistance / (referenceDistance + rolloffFactor * (distance - referenceDistance));
	return definedVolume * gain;
}
