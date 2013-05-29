/*
 *  SoundController.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "SoundController.h"

#ifdef __APPLE_CC__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif
#include <stdint.h>
#include <string>

#include "Controller.h"
#include "SoundBuffer.h"
#include "Robot.h"
#include "RobotSpeaker.h"

SoundController::SoundController(Controller *aController) : controller(aController)
{
	device = alcOpenDevice(NULL);
	if (!device) throw std::runtime_error("Could not open sound device.");
	context = alcCreateContext(device, NULL);
	if (!context) throw std::runtime_error("Could not create sound context.");
	
	alcMakeContextCurrent(context);
	
	alListenerf(AL_GAIN, 1.0f);
	
	capturedNoiseLevel = 0;
	
	// Sound capture
	captureDevice = alcCaptureOpenDevice(NULL, 44000, AL_FORMAT_MONO8, 2000);
	if (captureDevice) alcCaptureStart(captureDevice);
	isRecording = true;
}

void SoundController::addRobot(Robot *aRobot)
{
	RobotSpeaker *speaker = new RobotSpeaker(aRobot, this);
	aRobot->setSpeaker(speaker);
	speakers[aRobot] = speaker;
}

void SoundController::removeRobot(Robot *aRobot)
{
	aRobot->setSpeaker(NULL);
	speakers[aRobot] = NULL;
}


SoundController::~SoundController()
{
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
	if (captureDevice) alcCloseDevice(captureDevice);
}

SoundBuffer *SoundController::bufferWithFilename(const std::string &filename) throw()
{
	SoundBuffer *& buffer = loadedSoundFiles[filename];
	if (buffer == NULL)
	{
		try
		{
			buffer = new SoundBuffer(filename.c_str());
		}
		catch (std::exception e)
		{
			buffer = NULL;
		}
	}
	
	return buffer;	
}

void SoundController::update()
{
	alListenerfv(AL_POSITION, controller->getCameraLocation().c_ptr());
	
	// The camera stores its columns as four-component vectors, but OpenAL
	// requires them to have three only.
	const matrix &cameraMatrix = controller->getCameraMatrix();
	
	float4 forward = -cameraMatrix.z;
	float4 up = cameraMatrix.y;
	
	float orientation[6];
	memcpy(&(orientation[0]), forward.c_ptr(), sizeof(float [3]));
	memcpy(&(orientation[3]), up.c_ptr(), sizeof(float [3]));
	alListenerfv(AL_ORIENTATION, orientation);
	
	alListenerfv(AL_VELOCITY, controller->getCameraVelocity().c_ptr());
	
	if (captureDevice && isRecording)
	{
		ALCint numSamples;
		alcGetIntegerv(captureDevice, ALC_CAPTURE_SAMPLES, sizeof(numSamples), &numSamples);
		
		if (numSamples == 0) return;
		
		uint8_t *samples = new uint8_t[numSamples];
		
		alcCaptureSamples(captureDevice, samples, numSamples);
		
		uint8_t maxAmp = 128;
		for (ALCint i = 0; i < numSamples; i++)
		{
			if (samples[i] < 128) samples[i] = 255 - samples[i];
			if (samples[i] > maxAmp) maxAmp = samples[i];
		}
		
		delete [] samples;
		
		capturedNoiseLevel = float(maxAmp - 128) / 128.0f;
	}
}

float SoundController::noiseLevelAtPoint(const float4 &point) const throw()
{
	float totalLevelOfNoise = 0;
	
	for (std::map<Robot *, RobotSpeaker *>::const_iterator iter = speakers.begin(); iter != speakers.end(); ++iter)
		totalLevelOfNoise += iter->second->noiseLevelAtPoint(point);
	
	totalLevelOfNoise += capturedNoiseLevel;
	
	return totalLevelOfNoise;
}

void SoundController::setIsRecording(bool isit)
{
	if (isRecording == isit) return;
	
	isRecording = isit;
	
	if (!captureDevice) return;
	
	if (isRecording)
		alcCaptureStart(captureDevice);
	else
		alcCaptureStop(captureDevice);
}
