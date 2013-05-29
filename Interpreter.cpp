/*
 *  Interpreter.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "Interpreter.h"

#include "RXEFile.h"
#include "System.h"
#include "VMMemory.h"

Interpreter::Interpreter(const RXEFile *aFile, VMMemory *aMemory, System *aSystem)
: file(aFile), memory(aMemory), system(aSystem)
{
	instruction = 0;
	currentClump = 0;
	waitUntil = 0;
}

const char *Interpreter::nameForOpcode(unsigned opcode)
{
	switch (opcode)
	{
		case 0x00: return "OP_ADD";
		case 0x01: return "OP_SUB";
		case 0x02: return "OP_NEG";
		case 0x03: return "OP_MUL";
		case 0x04: return "OP_DIV";
		case 0x05: return "OP_MOD";
		case 0x36: return "OP_SQRT";
		case 0x37: return "OP_ABS";
		case 0x06: return "OP_AND";
		case 0x07: return "OP_OR";
		case 0x08: return "OP_XOR";
		case 0x09: return "OP_NOT";
		case 0x11: return "OP_CMP";
		case 0x12: return "OP_TST";
		case 0x15: return "OP_INDEX";
		case 0x16: return "OP_REPLACE";
		case 0x17: return "OP_ARRSIZE";
		case 0x18: return "OP_ARRBUILD";
		case 0x19: return "OP_ARRSUBSET";
		case 0x1A: return "OP_ARRINIT";
		case 0x1B: return "OP_MOV";
		case 0x1C: return "OP_SET";
		case 0x1D: return "OP_FLATTEN";
		case 0x1E: return "OP_UNFLATTEN";
		case 0x1F: return "OP_NUMTOSTRING";
		case 0x20: return "OP_STRINGTONUM";
		case 0x21: return "OP_STRCAT";
		case 0x22: return "OP_STRSUBSET";
		case 0x23: return "OP_STRTOBYTEARR";
		case 0x24: return "OP_BYTEARRTOSTR";
		case 0x25: return "OP_JMP";
		case 0x26: return "OP_BRCMP";
		case 0x27: return "OP_BRTST";
		case 0x29: return "OP_STOP";
		case 0x2A: return "OP_FINCLUMP";
		case 0x2B: return "OP_FINCLUMPIMMED";
		case 0x2C: return "OP_ACQUIRE";
		case 0x2D: return "OP_RELEASE";
		case 0x2E: return "OP_SUBCALL";
		case 0x2F: return "OP_SUBRET";
		case 0x28: return "OP_SYSCALL";
		case 0x30: return "OP_SETIN";
		case 0x31: return "OP_SETOUT";
		case 0x32: return "OP_GETIN";
		case 0x33: return "OP_GETOUT";
		case 0x34: return "OP_WAIT";
		case 0x35: return "OP_GETTICK";
		default: return "OP_UNKNOWN";
	}
}

void Interpreter::step()
{
	if (instruction >= file->getCodeWordCount()) return;
	
	if (system->getTick() < waitUntil)
		return;
	
	const uint16_t *code = file->getCode();
	
	if (code[instruction] & (1 << 11))
	{
		// Short format	
		unsigned opcode = (code[instruction] & 0x0700) >> 8;
		int argDiff = (int8_t) (code[instruction] & 0xFF);
		uint16_t arguments[2];
		switch(opcode)
		{
			case 0: // SHORT_OP_MOV
				arguments[0] = argDiff + code[instruction + 1];
				arguments[1] = code[instruction + 1];
				instruction += 2;
				op_mov(0, arguments);
				break;
			case 1: // SHORT_OP_ACQUIRE
				arguments[0] = argDiff;
				instruction += 1;
				op_acquire(0, arguments);
				break;
			case 2: // SHORT_OP_RELEASE
				arguments[0] = argDiff;
				instruction += 1;
				op_release(0, arguments);
				break;
			case 3: // SHORT_OP_SUBCALL
				arguments[0] = argDiff + code[instruction + 1]; // Subroutine
				arguments[1] = code[instruction + 1]; // Caller ID
				instruction += 2; // Instruction has to be on the next before
				// branching to subroutine so it can be used as return inst.
				op_subcall(0, arguments);
				break;
			default:
				throw std::runtime_error("Invalid short opcode.");
		}		
	}
	else
	{
		// Normal format
		unsigned opcode = code[instruction] & 0x00FF;
		// You’d think it’d be the other way around, but Lego’s documentation
		// tries to account for endianess and gets it all wrong.
		unsigned size = (code[instruction] & 0xF000) >> 12;	
		if (size == 0xE) size = code[instruction+1];
		unsigned flags = (code[instruction] & 0x0F00) >> 8;
		
		const uint16_t *params = &code[instruction + 1];
		
		instruction += size/2;
		
		switch(opcode)
		{	
			case 0x00: op_add(flags, params); break;
			case 0x01: op_sub(flags, params); break;
			case 0x02: op_neg(flags, params); break;
			case 0x03: op_mul(flags, params); break;
			case 0x04: op_div(flags, params); break;
			case 0x05: op_mod(flags, params); break;
			case 0x36: op_sqrt(flags, params); break;
			case 0x37: op_abs(flags, params); break;
			case 0x06: op_and(flags, params); break;
			case 0x07: op_or(flags, params); break;
			case 0x08: op_xor(flags, params); break;
			case 0x09: op_not(flags, params); break;
			case 0x11: op_cmp(flags, params); break;
			case 0x12: op_tst(flags, params); break;
			case 0x15: op_index(flags, params); break;
			case 0x16: op_replace(flags, params); break;
			case 0x17: op_arrsize(flags, params); break;
			case 0x18: op_arrbuild(flags, params); break;
			case 0x19: op_arrsubset(flags, params); break;
			case 0x1A: op_arrinit(flags, params); break;
			case 0x1B: op_mov(flags, params); break;
			case 0x1C: op_set(flags, params); break;
			case 0x1D: op_flatten(flags, params); break;
			case 0x1E: op_unflatten(flags, params); break;
			case 0x1F: op_numtostring(flags, params); break;
			case 0x20: op_stringtonum(flags, params); break;
			case 0x21: op_strcat(flags, params); break;
			case 0x22: op_strsubset(flags, params); break;
			case 0x23: op_strtobytearr(flags, params); break;
			case 0x24: op_bytearrtostr(flags, params); break;
			case 0x25: op_jmp(flags, params); break;
			case 0x26: op_brcmp(flags, params); break;
			case 0x27: op_brtst(flags, params); break;
			case 0x29: op_stop(flags, params); break;
			case 0x2A: op_finclump(flags, params); break;
			case 0x2B: op_finclumpimmed(flags, params); break;
			case 0x2C: op_acquire(flags, params); break;
			case 0x2D: op_release(flags, params); break;
			case 0x2E: op_subcall(flags, params); break;
			case 0x2F: op_subret(flags, params); break;
			case 0x28: op_syscall(flags, params); break;
			case 0x30: op_setin(flags, params); break;
			case 0x31: op_setout(flags, params); break;
			case 0x32: op_getin(flags, params); break;
			case 0x33: op_getout(flags, params); break;
			case 0x34: op_wait(flags, params); break;
			case 0x35: op_gettick(flags, params); break;

			default:
				{
					char opcodeString[255];
					sprintf(opcodeString, "Invalid opcode 0x%x size %d", opcode, size);
					throw std::invalid_argument(opcodeString);	
				}
		}
	}

}