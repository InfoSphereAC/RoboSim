/*
 *  VMMemory.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include "VMMemory.h"

#include <cstring>

#include "ByteOrder.h"
#include "RXEFile.h"

namespace
{
}

int32_t VMMemory::getScalar(RXEFile::dstocType type, const void *memoryLocation) const throw(std::invalid_argument)
{
	switch (type)
	{
			// Default
		case RXEFile::TC_UBYTE: return int32_t(SwapLittleToHost(reinterpret_cast<const uint8_t *>(memoryLocation)[0]));
		case RXEFile::TC_SBYTE: return int32_t(SwapLittleToHost(reinterpret_cast<const int8_t *>(memoryLocation)[0]));
		case RXEFile::TC_UWORD: return int32_t(SwapLittleToHost(reinterpret_cast<const uint16_t *>(memoryLocation)[0]));
		case RXEFile::TC_SWORD: return int32_t(SwapLittleToHost(reinterpret_cast<const int16_t *>(memoryLocation)[0]));
		case RXEFile::TC_ULONG: return int32_t(SwapLittleToHost(reinterpret_cast<const uint32_t *>(memoryLocation)[0]));
		case RXEFile::TC_SLONG: return int32_t(SwapLittleToHost(reinterpret_cast<const int32_t *>(memoryLocation)[0]));
			// Special cases
			
			// The field for an array contains an uint16_t which is the number of the dope vector.
		case RXEFile::TC_ARRAY: return int32_t(SwapLittleToHost(reinterpret_cast<const uint16_t *> (memoryLocation)[0]));
			
			// A mutex is a 32-bit value. What it contains is not fully clear.
		case RXEFile::TC_MUTEX: return int32_t(SwapLittleToHost(reinterpret_cast<const uint32_t *> (memoryLocation)[0]));
			
			// Extremely sketchy support for floats.
		case RXEFile::TC_FLOAT: return int32_t(SwapLittleToHost(reinterpret_cast<const float *> (memoryLocation)[0]));
			
			// We do NOT like clusters and voids
		default: throw std::invalid_argument("Invalid DSTOC type used for get");
	}
}

void VMMemory::setScalar(RXEFile::dstocType type, void *memoryLocation, int32_t value) throw(std::invalid_argument)
{
	switch (type)
	{
			// Default
		case RXEFile::TC_UBYTE:
			reinterpret_cast<uint8_t *>(memoryLocation)[0] = SwapHostToLittle(uint8_t(value));
			break;
		case RXEFile::TC_SBYTE:
			reinterpret_cast<int8_t *>(memoryLocation)[0] = SwapHostToLittle(int8_t(value));
			break;
		case RXEFile::TC_UWORD:
			reinterpret_cast<uint16_t *>(memoryLocation)[0] = SwapHostToLittle(uint16_t(value));
			break;
		case RXEFile::TC_SWORD:
			reinterpret_cast<int16_t *>(memoryLocation)[0] = SwapHostToLittle(int16_t(value));
			break;
		case RXEFile::TC_ULONG:
			reinterpret_cast<uint32_t *>(memoryLocation)[0] = SwapHostToLittle(uint32_t(value));
			break;
		case RXEFile::TC_SLONG:
			reinterpret_cast<int32_t *>(memoryLocation)[0] = SwapHostToLittle(int32_t(value));
			break;
			// Special cases
						
			// A mutex is a 32-bit value, containing whatever the system puts in there.
		case RXEFile::TC_MUTEX:
			reinterpret_cast<uint32_t *>(memoryLocation)[0] = SwapHostToLittle(uint32_t(value));
			break;
			
			// Extremely sketchy support for floats.
		case RXEFile::TC_FLOAT:
			reinterpret_cast<float *>(memoryLocation)[0] = SwapHostToLittle(float(value));
			break;
			
			// We do NOT like clusters and voids. We also do not allow writing to
			// the dstoc entry for an array. That just causes all sorts of
			// trouble.
		default: throw std::invalid_argument("Invalid DSTOC type used for get");
	}
	
}

VMMemory::VMMemory(const RXEFile *file) : programData(file)
{
	memory = new uint8_t [programData->getStaticSize()];
	
	// Find default value
	unsigned lastDefaultValuesEntry = 0;
	for (unsigned i = 0; i < programData->getDSTOCCount(); i++)
	{
		RXEFile::dstocType type = programData->getTypeAtDSTOCIndex(i);
		unsigned size = programData->getSizeOfType(type);
		
		if (type == RXEFile::TC_VOID || type == RXEFile::TC_CLUSTER) continue;
		
		if (programData->getFlagsAtDSTOCIndex(i) & 0x1)
		{
			// Uses default value of 0
			memset(memory + programData->getDataDescriptorAtDSTOCIndex(i), 0, size);
		}
		else
		{
			// Copy data over
			memcpy(memory + programData->getDataDescriptorAtDSTOCIndex(i), programData->getDefaultData() + lastDefaultValuesEntry, size);
			
			lastDefaultValuesEntry += size;
		}
		
		if (type == RXEFile::TC_ARRAY)
		{
			// Ignore contents of array
			RXEFile::dstocType nextType = programData->getTypeAtDSTOCIndex(i+1);
			while (nextType == RXEFile::TC_ARRAY)
			{
				// Skip over nested arrays
				i++;
				nextType = programData->getTypeAtDSTOCIndex(i+1);
			}
			if (nextType == RXEFile::TC_CLUSTER)
			{
				// Skip over cluster.
				// Contained arrays are accounted for in the Cluster’s size field.
				i += 1 + programData->getDataDescriptorAtDSTOCIndex(i + 1);
			}
			else
			{
				// Skip over plain element declaration
				i++;
			}
		}
	}
	
	// Prepare dynamic vectors
	
	// Difference in size between static data on the file and static data in memory.
	size_t staticSizeDiff = programData->getStaticSize() - programData->getDynamicDefaultOffset();
	
	// The start point for array offset. Add this to the offset in dope vectors
	// to find the array data.
	const uint8_t *arrayStartPoint = programData->getDefaultData() - staticSizeDiff;
	
	
	DopeVector *fileDopeVectors = (DopeVector *) (arrayStartPoint + programData->getDopeVectorOffset());
	
	// Prepare for resizing of dynamic vectors
	arrays = new char * [fileDopeVectors[0].elementCount];
    uint16_t arrayCount = SwapU16LittleToHost(fileDopeVectors[0].elementCount);
	for (unsigned i = 0; i < arrayCount; i++)
	{
        uint16_t elementCount = SwapU16LittleToHost(fileDopeVectors[i].elementCount);
		if (elementCount == 0)
		{
			arrays[i] = NULL;
		}
		else
		{
            uint16_t elementSize = SwapU16LittleToHost(fileDopeVectors[i].elementSize);
            uint16_t offset = SwapU16LittleToHost(fileDopeVectors[i].offset);
			arrays[i] = reinterpret_cast<char *>(malloc(elementSize * elementCount));
			memcpy(arrays[i], arrayStartPoint + offset, elementSize * elementCount);
		}
	}
	
	dopeVectors = reinterpret_cast<DopeVector *> (arrays[0]);
}

VMMemory::~VMMemory()
{
	delete [] memory;
	unsigned numArrays = dopeVectors[0].elementCount;
	for (unsigned i = 0; i < numArrays; i++)
		delete [] arrays[i];
	delete [] arrays;
}

int32_t VMMemory::getScalarValue(unsigned entry) const throw(std::range_error, std::invalid_argument)
{
	if (entry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	RXEFile::dstocType type = programData->getTypeAtDSTOCIndex(entry);
	return getScalar(type, memory + programData->getDataDescriptorAtDSTOCIndex(entry));
}
void VMMemory::setScalarValue(unsigned entry, int32_t newValue) throw(std::range_error, std::invalid_argument)
{
	if (entry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	RXEFile::dstocType type = programData->getTypeAtDSTOCIndex(entry);	
	setScalar(type, memory + programData->getDataDescriptorAtDSTOCIndex(entry), newValue);
}

int32_t VMMemory::getArrayElement(unsigned arrayDSTOC, unsigned arrayIndex)
{
	if (arrayDSTOC >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	
	// Check element type.
	RXEFile::dstocType elementType = programData->getTypeAtDSTOCIndex(arrayDSTOC + 1);
	if (elementType == RXEFile::TC_VOID || elementType == RXEFile::TC_CLUSTER) return -1;

	// Get dope vector
	int32_t dopeVector = getScalarValue(arrayDSTOC);
	if (arrayIndex >= SwapU16LittleToHost(dopeVectors[dopeVector].elementCount)) throw std::range_error("Array element not in range");
	
	// Get size of an element.
	unsigned size = SwapU16LittleToHost(dopeVectors[dopeVector].elementSize);
	
	return getScalar(elementType, &(arrays[dopeVector][size*arrayIndex]));
}

void VMMemory::setArrayElement(unsigned dstocEntry, unsigned arrayIndex, int32_t newValue)
{
	if (dstocEntry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	
	// Get element type (which is the DSTOC field right after the one for the
	// array)
	RXEFile::dstocType elementType = programData->getTypeAtDSTOCIndex(dstocEntry + 1);
	
	// Get dope vector
	int32_t dopeVector = getScalarValue(dstocEntry);
	if (arrayIndex >= SwapU16LittleToHost(dopeVectors[dopeVector].elementCount)) throw std::range_error("Array element not in range");
	
	// Get size of an element.
	unsigned size = SwapU16LittleToHost(dopeVectors[dopeVector].elementSize);
	
	// Set element
	setScalar(elementType, &(arrays[dopeVector][size*arrayIndex]), newValue);
}

unsigned VMMemory::getArrayLength(unsigned dstocEntry) const
{
	if (dstocEntry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	
	return SwapU16LittleToHost(dopeVectors[getScalarValue(dstocEntry)].elementCount);
}

void VMMemory::setArrayLength(unsigned dstocEntry, unsigned newLength)
{
	if (dstocEntry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	
	int32_t dopeVector = getScalarValue(dstocEntry);
	
	// Bail if the array size does not need to be changed.
	if (newLength == SwapU16LittleToHost(dopeVectors[dopeVector].elementCount)) return;
	
	unsigned newByteLength = newLength * SwapU16LittleToHost(dopeVectors[dopeVector].elementSize);
	arrays[dopeVector] = reinterpret_cast<char *>(realloc(arrays[dopeVector], newByteLength));
	dopeVectors[dopeVector].elementCount = SwapU16HostToLittle(newLength);
}

void *VMMemory::getArrayData(unsigned dstocEntry)
{
	if (dstocEntry >= programData->getDSTOCCount()) throw std::range_error("Not a DSTOC entry");
	
	int32_t dopeVector = getScalarValue(dstocEntry);
	
	return arrays[dopeVector];
}
