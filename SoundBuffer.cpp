/*
 *  SoundBuffer.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 04.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "SoundBuffer.h"

#include <fstream>
#include <stdint.h>
#include <string.h>
#ifdef __APPLE_CC__
#include <OpenAL/al.h>
#else
#include <al.h>
#endif

#include "ByteOrder.h"

void SoundBuffer::initWithData(const void *data, unsigned length, unsigned frequency, bool is16Bit, bool isStereo)
{
	alGenBuffers(1, &soundBufferID);
	ALenum format;
	if (is16Bit)
	{
		if (isStereo) format = AL_FORMAT_STEREO16;
		else format = AL_FORMAT_MONO16;
	}
	else
	{
		if (isStereo) format = AL_FORMAT_STEREO8;
		else format = AL_FORMAT_MONO8;
	}
	alBufferData(soundBufferID, format, data, length, frequency);
}

SoundBuffer::SoundBuffer(const char *filename) throw(std::runtime_error)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) throw std::runtime_error("Could not open sound file!");
		
	char actualGroupID[4];
	file.read(actualGroupID, 4);
	if (memcmp(actualGroupID, "RIFF", 4) != 0) throw std::runtime_error("Not a WAV file.");
	
	// File length minus eight. Ignored here.
	uint32_t fileLength;
	file.read(reinterpret_cast<char *> (&fileLength), 4);
    fileLength = SwapLittleToHost(fileLength);
	
	char actualRiffType[4];
	file.read(actualRiffType, 4);
	if (memcmp(actualRiffType, "WAVE", 4) != 0) throw std::runtime_error("Not a WAV file.");
	
	// Information about the file
	bool haveFmt = false;
	bool haveData = false;
	
	uint32_t dataLength;
	uint16_t numChannels;
	uint32_t samplesPerSecond;
	uint16_t bitsPerSample;
	char *data = NULL;
	
	// Scan chunks
	while (file.good() && !(haveFmt && haveData))
	{
		char chunkType[4];
		file.read(chunkType, 4);
		
		if (memcmp(chunkType, "fmt ", 4) == 0)
		{
			// Read Format header
			if (haveFmt) throw std::runtime_error("Format data specified more than once.");
			haveFmt = true;
			
			uint32_t chunkLength;
			file.read(reinterpret_cast<char *> (&chunkLength), 4);
            chunkLength = SwapLittleToHost(chunkLength);
			if (chunkLength != 16) throw std::runtime_error("Sound file format not supported.");
			
			uint16_t format;
			file.read(reinterpret_cast<char *> (&format), 2);
            format = SwapLittleToHost(format);
			if (format != 0x0001) throw std::runtime_error("Format is not PCM.");
			
			file.read(reinterpret_cast<char *> (&numChannels), 2);
            numChannels = SwapLittleToHost(numChannels);
			if (numChannels > 2) throw std::runtime_error("More than two channels.");
			
			file.read(reinterpret_cast<char *> (&samplesPerSecond), 4);
            samplesPerSecond = SwapLittleToHost(samplesPerSecond);
			
			// Bytes per second and block align.
			file.seekg(6, std::ios::cur);
			
			file.read(reinterpret_cast<char *> (&bitsPerSample), 2);
            bitsPerSample = SwapLittleToHost(bitsPerSample);
			if (bitsPerSample != 8 && bitsPerSample != 16) throw std::runtime_error("Bits per sample has unusual value.");
		}
		else if (memcmp(chunkType, "data", 4) == 0)
		{
			// Read actual data
			if (haveData) throw std::runtime_error("Sound data specified more than once.");
			haveData = true;
			
			file.read(reinterpret_cast<char *> (&dataLength), 4);
            dataLength = SwapLittleToHost(dataLength);
			data = new char [dataLength];
			file.read(data, dataLength);
		}
		else
		{
			// Skip unknown chunk
			uint32_t chunkLength;
			file.read(reinterpret_cast<char *> (&chunkLength), 4);
			chunkLength += chunkLength % 2; // RIFF-Files are word aligned
			file.seekg(chunkLength, std::ios::cur);
		}
	}
    
    if (bitsPerSample == 16)
        SwapU16LittleToHost(reinterpret_cast<uint16_t *>(data), dataLength/sizeof(uint16_t));
	
	initWithData(data, dataLength, samplesPerSecond, bitsPerSample == 16, numChannels == 2);
	
	delete [] data;
}

SoundBuffer::SoundBuffer(unsigned frequency, unsigned length)
{
	// The easiest way to do a square wave: Alternate min and max bytes for as
	// long as the buffer goes. Notice that we need to have twice the sampling
	// rate to make this work, in accordance with sampling rate laws in general.
	
	unsigned numSamples = (frequency * length * 2) / 1000 * 1000;
	uint8_t *samples = new uint8_t [numSamples];
	for (unsigned i = 0; i < numSamples/2; i++)
	{
		samples[i*2+0] = std::numeric_limits<uint8_t>::min();
		samples[i*2+1] = std::numeric_limits<uint8_t>::max();
	}
	
	initWithData(samples, numSamples, frequency*2, false, false);
	
	delete samples;
}

SoundBuffer::~SoundBuffer()
{
	alDeleteBuffers(1, &soundBufferID);
}
