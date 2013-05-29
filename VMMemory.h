/*
 *  VMMemory.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.04.10
 *  Copyright 2010 RWTH Aachen University All rights reserved.
 *
 */

#include <stdexcept>
#include <stdint.h>

#include "RXEFile.h"

#ifdef _MSC_VER
#define ATTRIBUTE_PACKED
#else
#define ATTRIBUTE_PACKED __attribute__((packed))
#endif

/*!
 * @abstract Memory of the virtual machine
 * @discussion An instance of this class manages the entire memory space of
 * a running Lego Mindstorms program. This means it contains all state that is
 * not system state, IO state or program counter.
 */
class VMMemory
{
	// Global data.
	const RXEFile *programData;
	
	// Stores the global scalars (which includes the dope vector numbers for
	// arrays, but not the arrays themselves)
	uint8_t *memory;
	
	// Meta-data for vectors, called Dope Vector by the Lego documentation for
	// some reason. In that documentation, a Dope Vector is an entry here, while
	// the entire array is the Dope Vector Array. Notice that this is a vector
	// itself, dopeVectors is aliased to arrays[0]. All fields except elementSize
	// and elementCount are unused and only included for compatibility with the
	// file format.. The actual data is contained in the entry of arrays with the
	// same index, i.e. the data for dopeVectors[i] is in arrays[i]
#ifdef _MSC_VER
#pragma pack (push, 1)
#endif
	struct DopeVector
	{
		uint16_t offset;
		uint16_t elementSize;
		uint16_t elementCount;
		uint16_t unused;
		uint16_t linkIndex;
	} ATTRIBUTE_PACKED;
#ifdef _MSC_VER
#pragma pack (pop)
#endif
	DopeVector *dopeVectors;
	
	// This actually contains the data.
	char **arrays;
	
	// Primitive methods to access any value, casting from/to the internal type.
	int32_t getScalar(RXEFile::dstocType type, const void *memoryLocation) const throw (std::invalid_argument);
	void setScalar(RXEFile::dstocType type, void *memoryLocation, int32_t newValue) throw (std::invalid_argument);
	
public:
	/*!
	 * @methodgroup Creating and destroying
	 */
	
	/*!
	 * @abstract Constructs a memory object
	 * @param file The RXE file to base this memory on. The file is used for the
	 * default data and the Dataspace Table of Contents (DSTOC) and has to exist
	 * for the entire time the memory does. The memory does not take ownership
	 * of the file, you have to delete it yourself after destroying the memory.
	 */
	VMMemory(const RXEFile *file);
	
	/*!
	 * @abstract Destructor
	 */
	~VMMemory();
	
	/*!
	 * @methodgroup Scalar value manipulation
	 */
	/*!
	 * @abstract Gets the value of a memory location
	 * @discussion This method is used for global scalars, as opposed to array
	 * elements. It automatically converts the value from the correct data type.
	 * It is also used to get the (read-only) dope vector of an array, which is
	 * just a normal memory value internally.
	 * @param entry The dataspace ID of the memory item to access.
	 * @result The memory item’s value.
	 * @throws std::invalid_argument if the type of the entry is one that cannot
	 * be read, that is cluster or void.
	 */	
	int32_t getScalarValue(unsigned entry) const throw(std::range_error, std::invalid_argument);
	
	/*!
	 * @abstract Sets a new value for a memory location
	 * @discussion This method is used for global scalars, as opposed to array
	 * elements. It automatically converts the value to the correct data type.
	 * You cannot use this to alter the dope vector of an array, because you
	 * generally should not do that.
	 * @param entry The dataspace ID of the memory item to alter.
	 * @param newValue The new value for this memory item.
	 * @throws std::invalid_argument if the type of the entry is one that cannot
	 * be written, that is array, cluster or void.
	 */
	void setScalarValue(unsigned entry, int32_t newValue) throw(std::range_error, std::invalid_argument);
	
	/*!
	 * @methodgroup Array manipulation
	 */
	/*!
	 * @abstract Returns the current length of an array. 
	 * @param dstocEntry The array identifier. NOT the Dope Vector, but the value
	 * passed as an argument to opcodes.
	 */
	unsigned getArrayLength(unsigned dstocEntry) const;
	
	/*!
	 * @abstract Sets the length of an array.
	 * @discussion Uses the same semantics as realloc: Truncates if newLength is
	 * shorter than the old length, otherwise the new elements are undefined.
	 * @param dstocEntry The array identifier. NOT the Dope Vector, but the value
	 * passed as an argument to opcodes.
	 * @param newLength The new length for the array (not bytes, elements).
	 */
	void setArrayLength(unsigned dstocEntry, unsigned newLength);
	
	/*!
	 * @abstract Returns an array element.
	 * @discussion Do not use this for complex arrays, i.e. arrays of clumps or
	 * arrays containing other arrays.
	 * @param dstocEntry The array identifier. NOT the Dope Vector, but the value
	 * passed as an argument to opcodes.
	 * @param offset The index of the entry, starting from 0.
	 * @result Whatever is stored at that entry, converted to an int32_t.
	 */
	int32_t getArrayElement(unsigned dstocEntry, unsigned offset);
	
	/*!
	 * @abstract Sets the value of an array element.
	 * @discussion Do not use this for complex arrays, i.e. arrays of clumps or
	 * arrays containing other arrays.
	 * @param dstocEntry The array identifier. NOT the Dope Vector, but the value
	 * passed as an argument to opcodes.
	 * @param offset The index of the entry, starting from 0.
	 * @param newValue The value to store. It will be casted to the right type
	 * automatically.
	 */
	void setArrayElement(unsigned dstocEntry, unsigned offset, int32_t newValue);
	
	/*!
	 * @abstract Returns the direct array data for an array
	 * @discussion This returns a pointer to internal data. You can edit it, but
	 * there is no bounds checking of any sort, and you must not delete it.
	 * @param dstocEntry The array identifier. NOT the Dope Vector, but the value
	 * passed as an argument to opcodes.
	 * @result The data of the array.
	 */
	void *getArrayData(unsigned dstocEntry);
};
