/*
 *  Interpreter_SystemIOInstructions.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 26.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Interpreter.h"

#include <ctime>
#include <iostream>

#include "RXEFile.h"
#include "System.h"
#include "VMMemory.h"

void Interpreter::op_syscall(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: SyscallID, immediate value
	// 1: ParmCluster, memory location
	
	unsigned syscallID = params[0];
	unsigned paramCluster = params[1];
	
	system->syscall(syscallID, paramCluster);
}

void Interpreter::op_setin(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Source, memory location
	// 1: Port, memory location
	// 2: PropID, immediate value
	
	unsigned source = memory->getScalarValue(params[0]);
	unsigned port = memory->getScalarValue(params[1]);
	unsigned propID = params[2];
	
	system->setInputConfiguration(port, propID, source);
}

void Interpreter::configureOutputForPort(unsigned port, unsigned numParams, const uint16_t *params)
{
	for (unsigned i = 0; i < numParams; i += 2)
		system->setOutputConfiguration(port, params[i], memory->getScalarValue(params[i+1]));
}

void Interpreter::op_setout(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Total size of instruction, immediate
	// 1: Port, memory location - either Array or Scalar
	// 2 + 2n + 0: PropID, immediate?
	// 2 + 2n + 1: Source, memory location?
	
	// params[0] is in bytes. Subtract instruction, inst.-size, port
	unsigned numParams = (params[0] / 2) - 3;
	
	RXEFile::dstocType portType = file->getTypeAtDSTOCIndex(params[1]);
	if (portType == RXEFile::TC_ARRAY)
	{
		for (unsigned i = 0; i < memory->getArrayLength(params[1]); i++)
		{
			unsigned port = memory->getArrayElement(params[1], i);
			configureOutputForPort(port, numParams, &params[2]);
		}
	}
	else
	{
		unsigned port = memory->getScalarValue(params[1]);
		configureOutputForPort(port, numParams, &params[2]);
	}
}

void Interpreter::op_getin(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location
	// 1: Port, memory location
	// 2: PropID, immediate value
	
	unsigned port = memory->getScalarValue(params[1]);
	unsigned property = params[2];
	
	unsigned result = system->getInputConfiguration(port, property);
	memory->setScalarValue(params[0], result);
}

void Interpreter::op_getout(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location
	// 1: Port, memory location
	// 2: PropID, immediate value
	
	unsigned port = memory->getScalarValue(params[1]);
	unsigned property = params[2];
	
	unsigned result = system->getOutputConfiguration(port, property);
	memory->setScalarValue(params[0], result);
}

void Interpreter::op_wait(unsigned flags, const uint16_t *params)
{
	// This operation is not specified by the newest firmware description
	// I could find, but NXC will emit it under some circumstances.
	// This description is hence mainly guesswork.
	
	// Params:
	// 0: Unknown. Seems to be NOT_A_DS_ID usually.
	// 1: Time to wait for, memory location
	
	waitUntil = system->getTick() + memory->getScalarValue(params[1]);
}

void Interpreter::op_gettick(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location
	
	memory->setScalarValue(params[0], system->getTick());
}
