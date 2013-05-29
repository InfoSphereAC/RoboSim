/*
 *  RXEFile.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "RXEFile.h"

#include <fstream>

#include "ByteOrder.h"

RXEFile::RXEFile(const char *filename) throw(std::runtime_error)
{
	std::ifstream file(filename, std::ios::binary);
	if (!file) throw std::runtime_error("Could not open RXE file.");
	
	// Read Header Data
	char formatstring[16];
	char expectedFormatString[16] = {'M', 'i', 'n', 'd', 's', 't', 'o', 'r', 'm', 's', 'N', 'X', 'T', 0, 0, 5 };
	file.read(formatstring, sizeof(formatstring));
	// Ignore final number, as others appear in the wild as well and appear to be compatible.
	if (memcmp(formatstring, expectedFormatString, 15) != 0) throw std::runtime_error("File lacks format header.");
	if (!file.good()) throw std::runtime_error("Unexpected end of file (after format header).");

	// Read Dataspace information
	uint16_t dataspaceHeader[9];
	file.read(reinterpret_cast<char *>(dataspaceHeader), sizeof(dataspaceHeader));
    SwapU16LittleToHost(dataspaceHeader, sizeof(dataspaceHeader)/sizeof(uint16_t));
	if (!file.good()) throw std::runtime_error("Unexpected end of file (after dataspace header).");
	dstocCount = (unsigned) dataspaceHeader[0];
	initialSize = (unsigned) dataspaceHeader[1];
	staticSize = (unsigned) dataspaceHeader[2];
	defaultDataSize = (unsigned) dataspaceHeader[3];
	dynamicDefaultOffset = (unsigned) dataspaceHeader[4];
	dynamicDefaultSize = (unsigned) dataspaceHeader[5];
	memoryManagerHead = (unsigned) dataspaceHeader[6];
	memoryManagerTail = (unsigned) dataspaceHeader[7];
	dopeVectorOffset = (unsigned) dataspaceHeader[8];
	
	// Read code information
	uint16_t codeSizeInformation[2];
	file.read(reinterpret_cast<char *>(codeSizeInformation), sizeof(codeSizeInformation));
    SwapU16LittleToHost(codeSizeInformation, sizeof(codeSizeInformation)/sizeof(uint16_t));
	if (!file.good()) throw std::runtime_error("Unexpected end of file (code size information).");
	clumpCount = (unsigned) codeSizeInformation[0];
	codeWordCount = (unsigned) codeSizeInformation[1];
	
	// Read DSTOC
	dstoc = new DSTOCEntry [dstocCount];
	file.read(reinterpret_cast<char *>(dstoc), sizeof(DSTOCEntry) * dstocCount);
	if (!file.good()) throw std::runtime_error("Unexpected end of file (after Dataspace table of contents).");
#if __BIG_ENDIAN__
    for (unsigned i = 0; i < dstocCount; i++)
        dstoc[i].descriptor = SwapU16LittleToHost(dstoc[i].descriptor);
#endif
	
	// Read default data
	defaultData = new uint8_t [defaultDataSize];
	file.read(reinterpret_cast<char *>(defaultData), defaultDataSize);
	if (!file.good()) throw std::runtime_error("Unexpected end of file (after default data).");
    // Default data is not swapped here, but by the memory.
	
    // Preserve alignment
	if ((std::streamoff(file.tellg()) % 2) != 0)
		file.seekg(1, std::ios::cur);
	
	// Read fixed-size part of clump records
	uint32_t *clumpDataBytes = new uint32_t [clumpCount];
	file.read(reinterpret_cast<char *>(clumpDataBytes), sizeof(uint32_t) * clumpCount);
	if (!file.good()) throw std::runtime_error("Unexpected end of file (after clump data).");
	
	// Read variable-size part of clump records
	clumpData = new ClumpData [clumpCount];
	for (unsigned i = 0; i < clumpCount; i++)
	{
		memcpy(&clumpData[i], &clumpDataBytes[i], sizeof(uint32_t));
		clumpData[i].dependents = new uint8_t [clumpData[i].dependentCount];
        clumpData[i].codeStartOffset = SwapU16LittleToHost(clumpData[i].codeStartOffset);
		file.read(reinterpret_cast<char *>(clumpData[i].dependents), clumpData[i].dependentCount);
		if (!file.good()) throw std::runtime_error("Unexpected end of file (in clump dependents data).");
	}
	
	// Next up: Actual code.
	code = new uint16_t [codeWordCount];
	file.read(reinterpret_cast<char *>(code), codeWordCount * sizeof(uint16_t));
    SwapU16LittleToHost(code, codeWordCount);
}

RXEFile::~RXEFile()
{
	delete [] dstoc;
	delete [] defaultData;
	for (unsigned i = 0; i < clumpCount; i++)
		delete [] clumpData[i].dependents;
	delete [] clumpData;
	delete [] code;
}

size_t RXEFile::getSizeOfType(dstocType type)
{
	switch(type)
	{
		case TC_UBYTE:
		case TC_SBYTE:
			return 1;
		case TC_UWORD:
		case TC_SWORD:
		case TC_ARRAY:
			return 2;
		case TC_ULONG:
		case TC_SLONG:
		case TC_MUTEX:
		case TC_FLOAT:
			return 4;
		default:
			return 0;
	}
}

const char *RXEFile::nameForType(RXEFile::dstocType type)
{
	switch (type)
	{
		case RXEFile::TC_VOID: return "void";
		case RXEFile::TC_UBYTE: return "ubyte";
		case RXEFile::TC_SBYTE: return "sbyte";
		case RXEFile::TC_UWORD: return "uword";
		case RXEFile::TC_SWORD: return "sword";
		case RXEFile::TC_ULONG: return "ulong";
		case RXEFile::TC_SLONG: return "slong";
			
		case RXEFile::TC_ARRAY: return "array";
		case RXEFile::TC_CLUSTER: return "clust";
		case RXEFile::TC_MUTEX: return "mutex";
		case RXEFile::TC_FLOAT: return "float";
			
		default: return "?????";
	}
	
}
