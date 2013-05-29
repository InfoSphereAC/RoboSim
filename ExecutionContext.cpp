/*
 *  ExecutionContext.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ExecutionContext.h"

#include <iostream>

#include "NetworkInterface.h"
#include "Interpreter.h"
#include "RXEFile.h"
#include "System.h"
#include "Time.h"
#include "VMMemory.h"

ExecutionContext::ExecutionContext(const char *aFilename) throw (std::runtime_error)
: filename(aFilename), file(0), memory(0), system(0), interpreter(0), networkInterface(0)
{
	if (!aFilename) throw std::runtime_error("Filename is NULL");
	
	isPaused = false;
	
	load();
}

ExecutionContext::~ExecutionContext()
{
	delete interpreter;
	delete system;
	delete memory;
	delete file;
}

void ExecutionContext::load()
{
	try {
		file = new RXEFile(filename.c_str());
		memory = new VMMemory(file);
		system = new System(memory);
		interpreter = new Interpreter(file, memory, system);		
	}
	catch (std::runtime_error e)
	{
		delete interpreter;
		delete system;
		delete memory;
		delete file;
		
		throw e;
	}
	
}

void ExecutionContext::reload()
{
	if (!this) return;
	
	delete interpreter;
	delete system;
	delete memory;
	delete file;
	
	load();
	
	if (networkInterface) system->setNetworkInterface(networkInterface);
}

void ExecutionContext::runForTime(float mintime)
{
	if (isPaused) return;
	
	unsigned long start = millisecondsSinceStart();
	
	if (interpreter->waitingUntilTick() > start)
		return;
	
	while ((float(millisecondsSinceStart()-start)/1000.0f) < mintime)
	{
		try
		{
			interpreter->step();
		}
		catch (std::exception e)
		{
			std::cout << "Exception during execution of code: " << e.what() << std::endl;
		}
	}
}

void ExecutionContext::setIsPaused(bool pause) throw()
{
	isPaused = pause;
	networkInterface->setIsPaused(isPaused);
}

void ExecutionContext::setNetworkInterface(NetworkInterface *anInterface)
{
	networkInterface = anInterface;
	system->setNetworkInterface(networkInterface);
}
