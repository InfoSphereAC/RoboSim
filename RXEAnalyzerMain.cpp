/*
 *  RXEAnalyzerMain.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <iostream>

#include "Interpreter.h"
#include "RXEFile.h"
#include "VMMemory.h"

void printUsageAndExit()
{
	std::cout << "Generates statistics from RXE files." << std::endl;
	std::cout << "Usage: RXEAnalyzer inputfile" << std::endl;
	exit(0);
}

void printComp(unsigned code)
{
	switch(code)
	{
		case 0:
			std::cout << "(<) ";
			break;
		case 1:
			std::cout << "(>) ";
			break;
		case 2:
			std::cout << "(<=) ";
			break;
		case 3:
			std::cout << "(>=) ";
			break;
		case 4:
			std::cout << "(==) ";
			break;
		case 5:
			std::cout << "(!=) ";
			break;
		default:
			std::cout << "(?) ";
			break;
	}
}

void printArgument(unsigned arg, RXEFile *file, bool isImmediate=false)
{
	std::cout << arg << " ";
	
	if (!isImmediate)
	{
		std::cout << "(";
		if (arg == 0xFFFF) std::cout << "NOT_A_DS_ID";
		else std::cout << file->nameForType(file->getTypeAtDSTOCIndex(arg));
		std::cout << ") ";
	}
	std::cout << " ";
}

int main(int argc, char *argv[])
{
	if (argc != 2) printUsageAndExit();
	
	RXEFile *file;
	
	try
	{
		file = new RXEFile(argv[1]);
	}
	catch (std::runtime_error a)
	{
		std::cerr << "Error opening file " << argv[1] << ": " << a.what() << std::endl;
		exit(-1);
	}
	
	// Print out the static header data.
	std::cout << "-- Static data --" << std::endl;
	std::cout << "DSTOC Count:            " << file->getDSTOCCount() << std::endl;
	std::cout << "Initial Size:           " << file->getInitialSize() << std::endl;
	std::cout << "Static Size:            " << file->getStaticSize() << std::endl;
	std::cout << "Default Data Size:      " << file->getDefaultDataSize() << std::endl;
	std::cout << "Dynamic Default Offset: " << file->getDynamicDefaultOffset() << std::endl;
	std::cout << "Dynamic Default Size:   " << file->getDynamicDefaultSize() << std::endl;
	std::cout << "Memory Manager Head:    " << file->getMemoryManagerHead() << std::endl;
	std::cout << "Memory Manager Tail:    " << file->getMemoryManagerTail() << std::endl;
	std::cout << "Dope Vector Offset:     " << file->getDopeVectorOffset() << std::endl;
	std::cout << "Clump Count:            " << file->getClumpCount() << std::endl;
	std::cout << "Code Word Count:        " << file->getCodeWordCount() << std::endl;
	
	// Create memory
	VMMemory *memory = new VMMemory(file);
	
	// Print out DSTOC and contents
	std::cout << std::endl;
	std::cout << "-- DSTOC --" << std::endl;
	std::cout << "entry\ttype\tflags\tdesc\tcontents" << std::endl;
	for (unsigned i = 0; i < file->getDSTOCCount(); i++)
	{
		std::cout << i << ":  \t" << file->nameForType(file->getTypeAtDSTOCIndex(i));
		std::cout << "\t" << file->getFlagsAtDSTOCIndex(i) << "\t\t" << file->getDataDescriptorAtDSTOCIndex(i);
		if (file->getTypeAtDSTOCIndex(i) != RXEFile::TC_CLUSTER)
			std::cout << "\t\t" << memory->getScalarValue(i);
		std::cout << std::endl;
	}
	
	// Print out dope vectors
	std::cout << std::endl;
	std::cout << "-- Dope Vectors --" << std::endl;
	std::cout << "entry\toffset\tsize\tcount\tlink" << std::endl;
	for (unsigned i = 0; i < memory->getArrayLength(0); i++)
	{
		std::cout << i << ":\t\t";
//		std::cout << memory->getArrayLength(i) << "\t\t";
		std::cout << std::endl;
	}
	
	// Print out clump data
	std::cout << std::endl;
	std::cout << "-- Clump data --" << std::endl;
	std::cout << "entry\tfc\tcs\tdependents" << std::endl;
	for (unsigned i = 0; i < file->getClumpCount(); i++)
	{
		std::cout << i << ":\t\t";
		std::cout << file->getFireCountForClump(i) << "\t";
		std::cout << file->getCodeStartForClump(i) << "\t";
		for (unsigned j = 0; j < file->getDependentCountForClump(i); j++)
			std::cout << file->getDependentsForClump(i)[j];
		std::cout << std::endl;
	}
	
	// Print out code
	std::cout << std::endl;
	std::cout << "-- Code --" << std::endl;
	std::cout << "pc\tword\tDisassembled" << std::endl;
	const uint16_t *code = file->getCode();
	unsigned codeWord = 0;
	unsigned instruction = 0;
	while (codeWord < file->getCodeWordCount())
	{
		std::cout << instruction << "\t" << codeWord << "\t\t";
		if (code[codeWord] & (1 << 11))
		{
			// Short format
			unsigned opcode = (code[codeWord] & 0x0700) >> 8;
			int argDiff = (int8_t) (code[codeWord] & 0xFF);
			switch(opcode)
			{
				case 0:
					std::cout << "SHORT_OP_MOV ";
					printArgument(argDiff + code[codeWord + 1], file);
					printArgument(code[codeWord + 1], file);
					codeWord += 2;
					break;
				case 1:
					std::cout << "SHORT_OP_ACQUIRE ";
					printArgument(argDiff, file);
					codeWord += 1;
					break;
				case 2:
					std::cout << "SHORT_OP_RELEASE ";
					printArgument(argDiff, file);
					codeWord += 1;
					break;
				case 3:
					std::cout << "SHORT_OP_SUBCALL ";
					printArgument(argDiff + code[codeWord + 1], file, true); // Subroutine
					printArgument(code[codeWord + 1], file); // Caller ID
					codeWord += 2;
					break;
			}
		}
		else
		{
			// Long format
			unsigned opcode = code[codeWord] & 0x00FF;
			// You’d think it’d be the other way around, but Lego’s documentation
			// tries to account for endianess and gets it all wrong.
			unsigned size = (code[codeWord] & 0xF000) >> 12;
			unsigned flags = (code[codeWord] & 0x0F00) >> 8;
			
			enum { ImmNone=0, ImmFirst=0x1, ImmLast=0x2, ImmBoth=0x3 } isImmediate = ImmNone;
			
			std::cout << Interpreter::nameForOpcode(opcode) << " ";
			
			// Special cases
			switch (opcode)
			{
				case 0x11: // OP_CMP
				case 0x12: // OP_TST
					printComp(flags);
					break;
				case 0x1C: // OP_SET
				case 0x30: // OP_SETIN
				case 0x32: // OP_GETIN
				case 0x33: // OP_GETOUT
					isImmediate = ImmLast;
					break;
				case 0x21: // OP_STRCAT
				case 0x25: // OP_JMP
				case 0x2B: // OP_FINCLUMPIMMED
				case 0x2E: // OP_SUBCALL
				case 0x28: // OP_SYSCALL
				case 0x31: // OP_SETOUT
					isImmediate = ImmFirst;
					break;
				case 0x26: // OP_BRCMP
				case 0x27: // OP_BRTST
					printComp(flags);
					isImmediate = ImmFirst;
					break;
				case 0x2A: // OP_FINCLUMP
					isImmediate = ImmBoth;
					break;
			}
			if (size == 0xE) size = code[codeWord+1];
			size /= 2;
			
			for (unsigned i = 1; i < size; i++)
				printArgument(code[codeWord+i], file, (i == 1 && (isImmediate & ImmFirst)) || (i == (size-1) && (isImmediate & ImmLast)));
			
			codeWord += size;
		}
		instruction += 1;
		std::cout << std::endl;
	}
	
	delete memory;
	delete file;
	
	return 0;
}