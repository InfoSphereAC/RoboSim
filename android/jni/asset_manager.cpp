//
//  asset_manager.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 07.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "asset_manager.h"

#include <android/log.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <zlib.h>

struct endOfCentralDirectory
{
	uint32_t signature;
	uint16_t discNumber;
	uint16_t startDisc;
	uint16_t numRecordsThisDisc;
	uint16_t numRecordsTotal;
	uint32_t centralDirectorySize;
	uint32_t centralDirectoryOffset;
	uint16_t commentLength;
} __attribute__((packed));

struct directoryFileHeader
{
	uint32_t signature;
	uint16_t versionMadeBy;
	uint16_t minExtractVersion;
	uint16_t bits;
	uint16_t compressionMethod;
	uint16_t modificationTime;
	uint16_t modificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t filenameLength;
	uint16_t extraFieldLength;
	uint16_t fileCommentLength;
	uint16_t discWhereFileStarts;
	uint16_t internalAttribs;
	uint32_t externalAttribs;
	uint32_t localHeaderOffset;
} __attribute__((packed));

struct localFileHeader
{
	uint32_t signature;
	uint16_t minExtractVersion;
	uint16_t bits;
	uint16_t compressionMethod;
	uint16_t modificationTime;
	uint16_t modificationDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t filenameLength;
	uint16_t extraFieldLength;
} __attribute__((packed));

AndroidAssetManager::AndroidAssetManager(const char *filename)
{
	__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Opening APK file %s", filename);
	
	struct stat statistics;
	stat(filename, &statistics);
	apkFileSize = statistics.st_size;
	
	apkFileDescriptor = open(filename, O_RDONLY);
	apkFileData = (char *) mmap(0, apkFileSize, PROT_READ, MAP_FILE | MAP_PRIVATE, apkFileDescriptor, 0);
	if (apkFileData == (char *) 0xFFFFFFFF)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Could not map file. errno %d", errno);
		exit(-2);
	}
	
	endOfCentralDirectory eocd;
	memcpy(&eocd, &(apkFileData[apkFileSize - 22]), 22);
	if (eocd.signature != 0x06054b50)
	{
		__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "EOCD signature is %x", eocd.signature);
		exit(-2);
	}

	
	size_t position = eocd.centralDirectoryOffset;
	for (unsigned i = 0; i < eocd.numRecordsTotal; i++)
	{
		directoryFileHeader dfh;
		
		// Read and check
		memcpy(&dfh, &(apkFileData[position]), 46);
		if (dfh.signature != 0x02014b50)
		{
			__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "dfh signature for entry %u is %x", i, dfh.signature);
			exit(-2);
		}
		
		// Find file name
		char *contentFileName = new char[dfh.filenameLength + 1];
		memcpy(contentFileName, &(apkFileData[position+46]), dfh.filenameLength);
		contentFileName[dfh.filenameLength] = 0;
		
		// Store in list
		directoryEntryForFile[contentFileName] = position;
		
		__android_log_print(ANDROID_LOG_VERBOSE, "librobosim.so", "Archive file %s starts at %d", contentFileName, dfh.localHeaderOffset);
		
		// Cleanup
		delete contentFileName;
		
		position += 46 + dfh.filenameLength + dfh.extraFieldLength + dfh.fileCommentLength;
	}
}

AndroidAssetManager::~AndroidAssetManager()
{
	close();
}

void AndroidAssetManager::close()
{
	munmap(apkFileData, apkFileSize);
	::close(apkFileDescriptor);
}

bool AndroidAssetManager::fileExists(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter = directoryEntryForFile.find(filename);
	return iter != directoryEntryForFile.end();
}

bool AndroidAssetManager::getFileIsCompressed(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter = directoryEntryForFile.find(filename);
	if (iter == directoryEntryForFile.end()) return false;
	
	const directoryFileHeader *header = reinterpret_cast<directoryFileHeader *>(&(apkFileData[iter->second]));
	return header->compressionMethod != 0;
}

uint32_t AndroidAssetManager::getFileSizeRaw(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter =directoryEntryForFile.find(filename);
	if (iter == directoryEntryForFile.end()) return 0;
	
	const directoryFileHeader *header = reinterpret_cast<directoryFileHeader *>(&(apkFileData[iter->second]));
	return header->compressedSize;
}

uint32_t AndroidAssetManager::getOriginalFileSize(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter =directoryEntryForFile.find(filename);
	if (iter == directoryEntryForFile.end()) return 0;
	
	const directoryFileHeader *header = reinterpret_cast<directoryFileHeader *>(&(apkFileData[iter->second]));
	return header->uncompressedSize;
}

const void * AndroidAssetManager::getRawFilePointer(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter =directoryEntryForFile.find(filename);
	if (iter == directoryEntryForFile.end()) return 0;
	
	const directoryFileHeader *header = reinterpret_cast<directoryFileHeader *>(&(apkFileData[iter->second]));
	const localFileHeader *localHeader = reinterpret_cast<localFileHeader *>(&(apkFileData[header->localHeaderOffset]));
	
	return &(apkFileData[header->localHeaderOffset + sizeof(localFileHeader) + localHeader->filenameLength + localHeader->extraFieldLength]);
}

void * AndroidAssetManager::getDecompressedCopy(const char *filename) const
{
	std::map<std::string, size_t>::const_iterator iter =directoryEntryForFile.find(filename);
	if (iter == directoryEntryForFile.end()) return 0;
	
	const directoryFileHeader *header = reinterpret_cast<directoryFileHeader *>(&(apkFileData[iter->second]));
	const localFileHeader *localHeader = reinterpret_cast<localFileHeader *>(&(apkFileData[header->localHeaderOffset]));
		
	if (header->compressionMethod != 0 && header->compressionMethod != 8) return 0;
	
	char *data = new char[header->uncompressedSize];
	const char *original = &(apkFileData[header->localHeaderOffset + sizeof(localFileHeader) + localHeader->filenameLength + localHeader->extraFieldLength]);
	__android_log_print(ANDROID_LOG_INFO, "librobosim.so", "Local file starts at %u", unsigned(original) - unsigned(apkFileData));
	
	if (header->compressionMethod == 0)
		memcpy(data, original, header->uncompressedSize);
	else
	{
		uLongf destinationLength = header->uncompressedSize;
		__android_log_print(ANDROID_LOG_INFO, "librobosim.so", "Calling with %p, %lu (%p), %p, %u", data, destinationLength, &destinationLength, original, header->compressedSize);
		int result = uncompress((Bytef *) data, &destinationLength, (const Bytef *) original, header->compressedSize);
		if (result != Z_OK)
		{
			if (result == Z_MEM_ERROR)
				__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Could not decompress %s: Not enough memory", filename);
			else if (result == Z_BUF_ERROR)
				__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Could not decompress %s: Output buffer too small", filename);
			else if (result == Z_DATA_ERROR)
				__android_log_print(ANDROID_LOG_ERROR, "librobosim.so", "Could not decompress %s: Compressed data corrupt", filename);
			delete [] data;
			return 0;
		}
	}
	
	return data;
}