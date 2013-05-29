/*
 *  AndroidSoundCountroller.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 20.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "AndroidSoundController.h"

#include "../../Controller.h"
#include "AndroidSoundBuffer.h"
#include "../../Robot.h"
#include "AndroidSpeaker.h"


SoundController::SoundController(Controller *aController) : controller(aController)
{
	//TODO: Implement sound on Android
	capturedNoiseLevel = 0;
}

void SoundController::addRobot(Robot *aRobot)
{
	//TODO: Implement sound on Android
	RobotSpeaker *speaker = new RobotSpeaker(aRobot, this);
	aRobot->setSpeaker(speaker);
	speakers[aRobot] = speaker;
}

void SoundController::removeRobot(Robot *aRobot)
{
	//TODO: Implement sound on Android
	aRobot->setSpeaker(NULL);
	speakers[aRobot] = NULL;
}


SoundController::~SoundController()
{
	//TODO: Implement sound on Android
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
	//TODO: Implement sound on Android
}

float SoundController::noiseLevelAtPoint(const float4 &point) const throw()
{
	//TODO: Implement sound on Android
	float totalLevelOfNoise = 0;
	
	for (std::map<Robot *, RobotSpeaker *>::const_iterator iter = speakers.begin(); iter != speakers.end(); ++iter)
		totalLevelOfNoise += iter->second->noiseLevelAtPoint(point);
	
	totalLevelOfNoise += capturedNoiseLevel;
	
	return totalLevelOfNoise;
}

void SoundController::setIsRecording(bool isit)
{
	//TODO: Implement sound on Android
}
