/*
 *  SoundBuffer.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

/*!
 * @abstract Represents audio content that can be played by sources.
 * @discussion A sound buffer stores sounds. It can be used by any number of 
 * sources at the same time.
 *
 * Much (in fact exactly) like objects that encapsulate OpenGL state, a
 * SoundBuffer has an implicit reference to an OpenAL context. This has to be
 * the same for any method called (including the destructor) and when passed
 * as an argument to other sound system methods as the one that was current
 * when the constructor was called.
 */
class SoundBuffer
{
	unsigned soundBufferID;
	void initWithData(const void *data, unsigned length, unsigned frequency, bool is16Bit, bool isStereo);
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
	unsigned getBufferID() const throw() { return soundBufferID; }
	
	/*! @abstract Destructor */
	~SoundBuffer();
};
