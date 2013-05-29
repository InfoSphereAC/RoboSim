/*
 *  RobotSpeaker.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

class Robot;
union float4;
class SoundBuffer;
class SoundController;

class RobotSpeaker
{
	Robot *robot;
	SoundBuffer *currentBuffer;
	SoundController *controller;
	
	bool deleteOldBufferWhenDone;
	
	unsigned sourceID;
	
	// Does the actual work for playTone and playFile. It is legal to pass a NULL
	// buffer, in which case the source’s buffer will be removed.
	void playSoundBuffer(SoundBuffer *buffer, bool loops, float volume, bool deleteWhenDone) throw();
	
public:
	RobotSpeaker(Robot *aRobot, SoundController *aController) throw();
	
	~RobotSpeaker();
	
	/*!
	 * @abstract Updates data that is dependent on robot state.
	 * @discussion That is mainly location and velocity, for proper 3D sound and
	 * doppler effect. One could argue that this is not technically needed for a
	 * small simulator, but it is fun and essentially free.
	 *
	 * Notice that this does not take a timedelta parameter; it has no use for
	 * such a thing.
	 */
	void update() throw();
	
	/*!
	 * @abstract Play a single tone for a while.
	 * @discussion If the robot is currently playing anything else, that will be
	 * aborted. The tone is a simple rectangle shape, not sine, mainly because
	 * that is both easier and what the original robot is doing as well.
	 * @param frequency The frequency of the tone.
	 * @param duration The duration of the tone, in milliseconds.
	 */
	void playTone(unsigned frequency, unsigned duration, bool loops, float volume) throw();
	
	/*!
	 * @abstract Play the contents of a sound file.
	 * @discussion The file has to be a WAV file supported by SoundBuffer.
	 * Using the SoundController, the file will not be loaded again if it had
	 * been loaded before.
	 */
	void playFile(const char *file, bool loops, float volume) throw();
	
	/*!
	 * @abstract Noise of the current speaker as seen from a point.
	 * @discussion This method only calculates distance attenuation based on
	 * the last specified volume. It does not listen to the sound at all, meaning
	 * that it reports the loudest it could be, not how loud it actually is.
	 */
	float noiseLevelAtPoint(const float4 &position) const throw();

	/*!
	 * @abstract The sound controller.
	 */
	SoundController *getSoundController() { return controller; }
};
