/*
 *  SoundController.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <map>
#include <string>

class Controller;
union float4;
class Robot;
class RobotSpeaker;
class SoundBuffer;

class SoundController
{
	Controller *controller;
	
	struct ALCdevice_struct *device;
	struct ALCdevice_struct *captureDevice;
	struct ALCcontext_struct *context;
	
	float capturedNoiseLevel;
	bool isRecording;
	
	std::map<std::string, SoundBuffer *> loadedSoundFiles;
	std::map<Robot *, RobotSpeaker *> speakers;
	
public:
	/*!
	 * @abstract Constructor
	 * @discussion The controller is required for position and speed data. You
	 * can pass NULL for this if you promise to never call update(), but it is
	 * not recommended.
	 * @param controller The controller object that controls the world this
	 * SoundController is placed in.
	 */
	SoundController(Controller *controller);
	
	void addRobot(Robot *aRobot);
	void removeRobot(Robot *aRobot);
	
	/*! Destructor */
	~SoundController();
	
	/*!
	 * @abstract Returns a buffer for a file.
	 * @discussion This returns either a buffer with the contents of a WAV file
	 * specified by filename, or, if that file does not exist or could not be
	 * opened, NULL. Either way, do not delete the result, it remains owned by
	 * the SoundController.
	 *
	 * The WAV file has to be compatible with the conditions given in the
	 * description of SoundBuffer, for obvious reasons.
	 * @param filename The file path.
	 * @result A buffer with that file, or NULL. Ownership of the buffer remains
	 * with the SoundController. Do not delete it.
	 */
	SoundBuffer *bufferWithFilename(const std::string &filename) throw();
	
	/*!
	 * @abstract Updates data dependent on other issues.
	 * @discussion The main purpose of this method is to set the velocity and
	 * position of the listener, to allow for 3D sound and doppler effects. This
	 * is, technically speaking, not at all necessary for this small simulator,
	 * but it’s essentially free and cool, so it gets included anyway.
	 *
	 * Notice that there is no timedelta parameter here, unlike other update()
	 * methods. No part of the calculations in here need it.
	 */
	void update();
	
	/*!
	 * @abstract Calculates the noise level at a particular point.
	 * @discussion This method uses all the values and formulas that OpenAL
	 * uses, but notably does not listen to any playing sound data. Instead,
	 * it just uses the source gain.
	 */
	float noiseLevelAtPoint(const float4 &point) const throw();
	
	/*!
	 * @abstract Sets whether recording should take place.
	 * @discussion Recording is used to calculate the noise level. When the
	 * application is in the background, paused, or similar, it should be
	 * disabled, mostly because otherwise, there's the confusing red bar on
	 * iOS devices.
	 */
	void setIsRecording(bool isit);
};
