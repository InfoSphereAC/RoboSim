/*
 *  Interpreter_CompareInstructions.cpp
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

bool Interpreter::compare(unsigned mode, int a, int b)
{
	switch (mode)
	{
		case 0: // <
			return a < b;
		case 1: // >
			return a > b;
		case 2: // <=
			return a <= b;
		case 3: // >=
			return a >= b;
		case 4: // ==
			return a == b;
		case 5: // !=
			return a != b;
		default:
			throw std::runtime_error("Unknown compare mode");
	}
}

void Interpreter::op_cmp(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location - store result here
	// 1: Source1, memory location
	// 2: Source2, memory location
	int a = memory->getScalarValue(params[1]);
	int b = memory->getScalarValue(params[2]);
	memory->setScalarValue(params[0], compare(flags, a, b));
}

void Interpreter::op_tst(unsigned flags, const uint16_t *params)
{
	// Params:
	// 0: Destination, memory location - store result here
	// 1: Source, memory location
	int a = memory->getScalarValue(params[1]);
	memory->setScalarValue(params[0], compare(flags, a, 0));
}
