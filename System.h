/*
 *  System.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 26.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdint.h>

class NetworkInterface;
class VMMemory;

class System
{
	NetworkInterface *networkInterface;
	VMMemory *memory;
	
	uint8_t lowspeedOutputBuffer[16];
	int bytesReady;
	
	// Debug only
	static const char *nameForInputPartID(unsigned ID);
	static const char *nameForOutputPartID(unsigned ID);
	static const char *nameForSyscall(unsigned ID);
	
	// Communication (mainly with ultrasound sensor)
	int CheckLSStatus(int port, int &bytesReady);
	int LSRead(int port, uint8_t *buffer, int bufferLength);
	int LSWrite(int port, const uint8_t *buffer, int bufferLength, int bytesExpectedBack);
	
	// Sound support: Makes sure no invalid filenames get used.
	bool sanitizeSoundFilename(unsigned dstocEntry, char *bufferOut);
	
public:
	System(VMMemory *someMemory) : networkInterface(0), memory(someMemory) {}
	
	void setNetworkInterface(NetworkInterface *anInterface) { networkInterface = anInterface; };
	
	// Called by interpreted code
	void syscall(unsigned callID, unsigned paramClusterDSTOCIndex);
	
	void setInputConfiguration(unsigned port, unsigned property, unsigned value);
	unsigned getInputConfiguration(unsigned port, unsigned property);
	
	void setOutputConfiguration(unsigned port, unsigned property, unsigned value);
	int getOutputConfiguration(unsigned port, unsigned property);
	
	unsigned getTick();
};
