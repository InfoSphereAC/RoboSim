/*
 *  Interpreter_DataManipulationInstructions.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 26.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Interpreter.h"

#include <iostream>

#include "RXEFile.h"
#include "VMMemory.h"

void Interpreter::op_index(unsigned flags, const uint16_t *params)
{
	// Store element of array in scalar
	// Params:
	// 0: Destination, scalar, memory location
	// 1: Source, array, memory location
	// 2: Index, scalar, memory location or NOT_DS_ID (then it defaults to 0)
	
	unsigned index = 0;
	if (params[2] != 0xFFFF) index = memory->getScalarValue(params[2]);
	
	memory->setScalarValue(params[0], memory->getArrayElement(params[1], index));
}

void Interpreter::op_replace(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored replace" << std::endl;
}

void Interpreter::op_arrsize(unsigned flags, const uint16_t *params)
{
	// Stores size of array in scalar
	// Params:
	// 0: Destination, scalar, memory location
	// 1: Source, array, memory location

	memory->setScalarValue(params[0], memory->getArrayLength(params[1]));
}

void Interpreter::op_arrbuild(unsigned flags, const uint16_t *params)
{
	// Builds an array out of its parameters
	// 0: Total size of instruction, immediate
	// 1: destination
	// n: Data
	
	// params[0] is in bytes. Subtract word for instruction, inst.size and destination
	unsigned numElements = (params[0] / 2) - 3;
	
	memory->setArrayLength(params[1], numElements);
	for (unsigned i = 0; i < numElements; i++)
		memory->setArrayElement(params[1], i, memory->getScalarValue(params[2 + i]));
}

void Interpreter::op_arrsubset(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored arrsubset" << std::endl;
}

void Interpreter::op_arrinit(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination array, memory location
	// 1: New value, memory location
	// 2: Count, memory location
	
	int32_t newVal = memory->getScalarValue(params[1]);
	unsigned count = 0;
	if (params[2] != 0xFFFF) count = memory->getScalarValue(params[2]);
	
	memory->setArrayLength(params[0], count);
	for (unsigned i = 0; i < count; i++)
		memory->setArrayElement(params[0], i, newVal);
}

void Interpreter::op_mov(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location
	// 1: Source, memory location
	
	RXEFile::dstocType destType = file->getTypeAtDSTOCIndex(params[0]);
	RXEFile::dstocType srcType = file->getTypeAtDSTOCIndex(params[1]);
	
	if (destType == RXEFile::TC_ARRAY && srcType == RXEFile::TC_ARRAY)
	{
		// Array move
		unsigned newLength = memory->getArrayLength(params[1]);
		memory->setArrayLength(params[0], newLength);
		for (unsigned i = 0; i < newLength; i++)
			memory->setArrayElement(params[0], i, memory->getArrayElement(params[1], i));
	}
	else if ((destType == RXEFile::TC_VOID || destType == RXEFile::TC_CLUSTER || destType == RXEFile::TC_ARRAY) || (srcType == RXEFile::TC_VOID || srcType == RXEFile::TC_ARRAY || srcType == RXEFile::TC_CLUSTER || srcType == RXEFile::TC_ARRAY))
	{
		std::cout << "ignored complex mov from type " << file->nameForType(srcType) << " to type " << file->nameForType(destType) << std::endl;
		return;
	}
	else // Scalar move
		memory->setScalarValue(params[0], memory->getScalarValue(params[1]));
}

void Interpreter::op_set(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: location (memory location)
	// 1: value (immediate)
	
	memory->setScalarValue(params[0], params[1]);
}

void Interpreter::op_flatten(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored flatten" << std::endl;
}

void Interpreter::op_unflatten(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored unflatten" << std::endl;
}

void Interpreter::op_numtostring(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored numtostring" << std::endl;
}

void Interpreter::op_stringtonum(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored stringtonum" << std::endl;
}

void Interpreter::op_strcat(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored strcat" << std::endl;
}

void Interpreter::op_strsubset(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored strsubset" << std::endl;
}

void Interpreter::op_strtobytearr(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored strtobytearr" << std::endl;
}

void Interpreter::op_bytearrtostr(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored bytearrtostr" << std::endl;
}

