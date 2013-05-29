/*
 *  Interpreter_ControlFlowInstructions.cpp
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

void Interpreter::op_jmp(unsigned flags, const uint16_t *params)
{
	// Parameters:
	// 0: offset (immediate, signed): Alter PC by this much.
	
	int16_t offset = int16_t(params[0]);
	instruction += (offset - 2);
	// Account for size of jmp instruction, as otherwise we will go right past it.
}

void Interpreter::op_brcmp(unsigned flags, const uint16_t *params)
{
	// Parameters:
	// 0: offset (immediate, signed): Alter PC by this much if comparison is true
	// 1: Source1, memory location
	// 2: Source2, memory location
	
	int a = memory->getScalarValue(params[1]);
	int b = memory->getScalarValue(params[2]);
	if (compare(flags, a, b))
	{
		int16_t offset = int16_t(params[0]);
		instruction += (offset - 4);
		// Account for size of instruction, as otherwise we will go right past it.
	}
}

void Interpreter::op_brtst(unsigned flags, const uint16_t *params)
{
	// Parameters:
	// 0: offset (immediate, signed): Alter PC by this much if comparison is true
	// 1: Source, memory location
	
	int a = memory->getScalarValue(params[1]);
	if (compare(flags, a, 0))
	{
		int16_t offset = int16_t(params[0]);
		instruction += (offset - 3);
		// Account for size of instruction, as otherwise we will go right past it.
	}
}

void Interpreter::op_stop(unsigned flags, const uint16_t *params)
{
	throw std::runtime_error("op_stop");
}

void Interpreter::op_finclump(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored finclump" << std::endl;
}

void Interpreter::op_finclumpimmed(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored finclumpimmed" << std::endl;
}

void Interpreter::op_acquire(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored acquire" << std::endl;
}

void Interpreter::op_release(unsigned flags, const uint16_t *params)
{
	std::cout << "ignored release" << std::endl;
}

void Interpreter::op_subcall(unsigned flags, const uint16_t *params)
{
	// Parameters:
	// 0: Subroutine, immediate. Clump to branch to.
	// 1: CallerID, memory. Store ID of this clump there.
	
	// Store clump to jump back to.
	memory->setScalarValue(params[1], currentClump);
	// Store instruction to jump back to
	stack.push_back(instruction);
	
	// Branch into new clump
	currentClump = params[0];
	instruction = file->getCodeStartForClump(currentClump);
}

void Interpreter::op_subret(unsigned flags, const uint16_t *params)
{
	// Parameters:
	// 0: CallerID, memory. Jump back to clump with this ID.
	
	// Find old clump and restore operation there.
	currentClump = memory->getScalarValue(params[0]);
	instruction = stack.back();
	stack.pop_back();
}
