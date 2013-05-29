//
//  asset_manager.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 06.04.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef ANDROID_ASSET_MANAGER_H
#define ANDROID_ASSET_MANAGER_H

#include <map>
#include <string>

class AndroidAssetManager
{
	char *apkFileData;
	size_t apkFileSize;
	int apkFileDescriptor;
	
	std::map<std::string, size_t> directoryEntryForFile;
	
	void close();
public:
	AndroidAssetManager(const char *apkFilename);
	~AndroidAssetManager();
	
	/*!
	 * Whether the given file exists in the archive
	 */
	bool fileExists(const char *filename) const;
	
	/*!
	 * Whether the file is compressed.
	 */
	bool getFileIsCompressed(const char *filename) const;
	
	/*!
	 * The file size as stored in the archive.
	 */
	uint32_t getFileSizeRaw(const char *filename) const;
	
	/*!
	 * The decompressed file size. For files that are not compressed, this
	 * will be the same as the raw file size.
	 */
	uint32_t getOriginalFileSize(const char *filename) const;
	
	/*!
	 * A pointer to the raw file data, compressed or not.
	 * @discussion This is a direct pointer to the internal storage, so do not
	 * modify, free() or delete it.
	 */
	const void * getRawFilePointer(const char *filename) const;
	
	/*!
	 * A copy of the uncompressed data of a file.
	 * @discussion If the file was not compressed, this method will simply
	 * return a copy of the file's data. When you're done with it, delete it.
	 * If this function does not understand the compression method, it will
	 * return 0.
	 */
	void * getDecompressedCopy(const char *filename) const;
};

#endif /* ANDROID_ASSET_MANAGER_H */