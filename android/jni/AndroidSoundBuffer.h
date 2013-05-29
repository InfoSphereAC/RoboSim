/*
 *  AndroidSoundBuffer.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 20.01.11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

class SoundBuffer
{
public:
	/*!
	 * @abstract Creates a SoundBuffer from a WAV file.
	 * @discussion Only WAV files with at most two channels, 8 or 16 bit
	 * depth and linear PCM are supported.
	 * @param WAVfilename The path to the file to load.
	 * @throws std::runtime_error If the file cannot be opened or is not a valid
	 * WAV file.
	 */
	SoundBuffer(const char *WAVfilename) throw(std::runtime_error);
	
	/*!
	 * @abstract Creates a sound buffer with a single frequency.
	 * @discussion  This creates a simple rectangle signal, nothing fancy at all.
	 * It could easily to create a sine wave, but as far as my iPod-damaged ears
	 * can tell the NXT doesn’t do that itself.
	 * @param frequency The frequency to generate
	 * @param time The number of milliseconds this tone should be played.
	 */
	SoundBuffer(unsigned frequency, unsigned time);
	
	/*!
	 * @abstract Returns the sound buffer ID
	 * @discussion For OpenAL, this is the plain sound buffer ID. Do not delete
	 * it.
	 */
	unsigned getBufferID() const throw() { return 0; }
	
	/*! @abstract Destructor */
	~SoundBuffer();
};
