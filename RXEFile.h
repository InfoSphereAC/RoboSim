#pragma once
/*
 *  RXEFile.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdexcept>
#include <stdint.h>

/*!
 * @abstract Encapsulates the contents of an RXE file
 * @discussion An RXE file contains all the information for one robot program.
 * The most important parts are: 
 * 
 * 1. Location and data type of the memory fields used by the program.
 * 2. Default data for these memory fields.
 * 3. The actual executable bytecode.
 * 
 * It is an immutable object, you cannot alter it once all data has been loaded.
 * There ought to be no problem accessing it in parallel from multiple threads,
 * but this is not done in this program.
 */

#ifdef _MSC_VER
#define ATTRIBUTE_PACKED
#else
#define ATTRIBUTE_PACKED __attribute__((packed))
#endif

class RXEFile
{
	unsigned dstocCount;
	unsigned initialSize;
	unsigned staticSize;
	unsigned defaultDataSize;
	unsigned dynamicDefaultOffset;
	unsigned dynamicDefaultSize;
	unsigned memoryManagerHead;
	unsigned memoryManagerTail;
	unsigned dopeVectorOffset;
	
	unsigned clumpCount;
	unsigned codeWordCount;
#ifdef _MSC_VER
#pragma pack (1)
#endif
	struct DSTOCEntry
	{
		uint8_t type;
		uint8_t flags;
		uint16_t descriptor;
	} ATTRIBUTE_PACKED;

#ifdef _MSC_VER
#pragma pack (1)
#endif
	struct ClumpData
	{
		uint8_t fireCount;
		uint8_t dependentCount;
		uint16_t codeStartOffset;
		uint8_t *dependents;
	} ATTRIBUTE_PACKED;
	
	DSTOCEntry *dstoc;
	uint8_t *defaultData;
	ClumpData *clumpData;
	uint16_t *code;
public:
	/*!
	 * @abstract A datatype used in the dataspace table of contents (DSTOC)
	 * @discussion Any program-accessible data is one of these types or a
	 * combination of them.
	 * @constant TC_VOID Unused data, no size.
	 * @constant TC_UBYTE Unsigend 8 bit value.
	 * @constant TC_SBYTE Signed 8 bit value.
	 * @constant TC_UWORD Unsigend 16 bit value.
	 * @constant TC_SWORD Signed 16 bit value.
	 * @constant TC_ULONG Unsigend 32 bit value.
	 * @constant TC_SLONG Signed 32 bit value.
	 * @constant TC_ARRAY An array of other elements. In the DSTOC, the next
	 * entry specifies the type of the elements (which can be anything, including
	 * a cluster and another array). An array also has a normal associated 16-bit
	 * value that you can access the same way as a TC_UWORD, which is the index
	 * into the Dope Vector Array, a data structrue that which describes the
	 * array further.
	 * @constant TC_CLUSTER A cluster of other elements. The value in the DSTOC
	 * specifies how many elements follow. Anything can be part of a cluster.
	 * Operations can access both the cluster as a whole and individual parts.
	 * @constant TC_MUTEX A mutex, which is a system-assigned 32 bit value that
	 * is used together with special instructions to provide synchronisation.
	 * @constant TC_FLOAT A 32-bit float. Not really used much anywhere.
	 */
	enum dstocType {
		TC_VOID = 0,
		TC_UBYTE = 1,
		TC_SBYTE = 2,
		TC_UWORD = 3,
		TC_SWORD = 4,
		TC_ULONG = 5,
		TC_SLONG = 6,
		TC_ARRAY = 7,
		TC_CLUSTER = 8,
		TC_MUTEX = 9,
		TC_FLOAT = 10
	};
	
	/*!
	 * @abstract Gets the size of an entry of a given type.
	 * @param type The type.
	 * @result The size, as given by sizeof().
	 */
	static size_t getSizeOfType(dstocType type);
	
	/*!
	 * @abstract Constructs an RXE File object.
	 * @filename The name of the file to open it from. The file contents are
	 * copied, it can be altered or deleted after this method returns.
	 * @throws std::runtime_error if the file cannot be opened or is corrupt in
	 * some way.
	 */
	RXEFile(const char *filename) throw(std::runtime_error);
	
	/*!
	 * @abstract Destructor
	 */
	~RXEFile();
	
	/*! Number of entries in the Dataspace Table of Contents. */
	unsigned getDSTOCCount() const { return dstocCount; }
	/*! Initial size of the programs memory space. */
	unsigned getInitialSize() const { return initialSize; }
	/*! Amount of memory taken up by scalar values (anything except arrays). */
	unsigned getStaticSize() const { return staticSize; }
	/*! Size of the default data stored in the file, which is slighty compressed and hence less than when expanded. */
	unsigned getDefaultDataSize() const { return defaultDataSize; }
	/*! Offset to the array default data from the start of all default data. */
	unsigned getDynamicDefaultOffset() const { return dynamicDefaultOffset; }
	/*! Size of the array default data. */
	unsigned getDynamicDefaultSize() const { return dynamicDefaultSize; }
	/*! First entry in the memory manager linked list, which manages arrays in the order of their address. */
	unsigned getMemoryManagerHead() const { return memoryManagerHead; }
	/*! Last entry in the memory manager linked list, which orders arrays in the order of their address. */
	unsigned getMemoryManagerTail() const { return memoryManagerTail; }
	/*! Offset of the dope vector array from the beginning of the expanded memory. */
	unsigned getDopeVectorOffset() const { return dopeVectorOffset; }
	
	/*! Returns the amount of clumps (both tasks and subroutines) in the program. */
	unsigned getClumpCount() const { return clumpCount; }
	/*! Returns the number of words used by the program code. */
	unsigned getCodeWordCount() const { return codeWordCount; }
	
	/*! Type of a Dataspace Table of Contents entry. */
	dstocType getTypeAtDSTOCIndex(unsigned index) const { return dstocType(dstoc[index].type); }
	/*!
	 * @abstract Flags of a dataspace table of contents entry.
	 * @discussion Currently only one flag is defined: 0x1 means that instead of
	 * an entry in the default data, the initial value for this entry is 0.
	 */
	unsigned getFlagsAtDSTOCIndex(unsigned index) const { return dstoc[index].flags; }
	/*!
	 * @abstract Data descriptor of a dataspace table of contents entry.
	 * @discussion For a scalar value, this is the address where the value can
	 * be found. For an array, this is the adresss where its dope vector array
	 * index can be found. For a clump, this is the number of elements it
	 * contains.
	 */
	unsigned getDataDescriptorAtDSTOCIndex(unsigned index) const { return dstoc[index].descriptor; }
	
	/*! Default data stored in the file (not expanded in any way, always little-endian) */
	const uint8_t *getDefaultData() const { return defaultData; }
	
	/*! Fire count for a clump. */
	unsigned getFireCountForClump(unsigned clump) const { return clumpData[clump].fireCount; }
	/*! Number of dependents of a clump. */
	unsigned getDependentCountForClump(unsigned clump) const { return clumpData[clump].dependentCount; }
	/*! Start of the code of a clump. */
	unsigned getCodeStartForClump(unsigned clump) const { return clumpData[clump].codeStartOffset; }
	/*! Dependents of a clump, will be started when the clump ends. */
	const uint8_t *getDependentsForClump(unsigned clump) const { return clumpData[clump].dependents; }
	
	/*! Actual code data */
	const uint16_t *getCode() const { return code; }
	
	// Debug
	/*! Name of a type used in the Dataspace Table of Contents, or "?????" if not a valid type. */
	static const char *nameForType(dstocType type);
};
