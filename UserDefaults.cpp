//
//  UserDefaults.c
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.11.10.
//  Copyright (c) 2010 __MyCompanyName__. All rights reserved.
//

#include "UserDefaults.h"

#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#include <fstream>

#elif defined(__APPLE_CC__)
#include <CoreFoundation/CoreFoundation.h>
#else
// TODO: Find a way to do this on android
#endif

#ifdef _WIN32
const char *registryPath = "Software\\RoboSim";
#endif

void setDataForUserInterfaceKey(const char *keyName, unsigned dataSize, void *data)
{
#ifdef _WIN32
	// Save in File
	std::ofstream f("current_map.bin", std::ios::binary);
	if (f.is_open()) {
		f.write( (char *)data, dataSize);
	}
	f.close();

	// Save in registry
	HKEY hKey;
	
	// Open the key handle
	if(RegCreateKeyExA(HKEY_CURRENT_USER, registryPath, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL) != ERROR_SUCCESS) throw std::runtime_error("Could not open key");
	
	// Set the value
	if (RegSetValueExA(hKey, LPCSTR(keyName), 0, REG_BINARY, (LPBYTE) data, dataSize) != ERROR_SUCCESS) throw std::runtime_error("Could not set value");
	
	// Save the key handle and close it.
	RegFlushKey(hKey);
	RegCloseKey(hKey);
	
#elif defined(__APPLE_CC__)
	CFStringRef keyString = CFStringCreateWithCString(kCFAllocatorDefault, keyName, kCFStringEncodingASCII);
	
	CFDataRef dataValue = CFDataCreate(kCFAllocatorDefault, (const UInt8 *) data, (CFIndex) dataSize);
	
	CFPreferencesSetAppValue(keyString, dataValue, kCFPreferencesCurrentApplication);
	
	CFRelease(keyString);
	CFRelease(dataValue);
	
	CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
	
#else
	// TODO: Find a way to do this on android
#endif
}

void *getDataAndSizeForUserInterfaceKey(const char *keyName, unsigned &outDataSize)
{
#ifdef _WIN32
	// Load from file
			
	std::ifstream file;
	file.open("ressources\\current_map.bin", std::ios::binary | std::ios::ate);
	if (file.is_open()){

		// Find size
		std::streampos file_size;
		file_size = file.tellg();

		//Create buffer
		outDataSize = file_size;
		char *output_file = new char[outDataSize];

		//Get actual value
		file.seekg(0, std::ios::beg);
		file.read(output_file, outDataSize);

		//close file
		file.close();
		return output_file;
	}
	else return NULL;

			
	//// Load from registry
	//HKEY hKey;
	//
	//// Open key
	//if(RegCreateKeyExA(HKEY_CURRENT_USER, registryPath, 0, NULL, 0, KEY_READ, NULL, &hKey, NULL) != ERROR_SUCCESS) throw std::runtime_error("Could not open key");
	//
	//// Find size
	//DWORD dwSize = 0;
	//DWORD dwType = REG_BINARY;
	//if (RegQueryValueExA(hKey, LPCSTR(keyName), 0, &dwType, (LPBYTE) NULL, &dwSize) != ERROR_SUCCESS)
	//{
	//	RegCloseKey(hKey);
	//	return NULL;
	//}
	//
	//// Create buffer
	//outDataSize = dwSize;
	//char *output = new char [outDataSize];
	//
	//// Get actual value
	//if (RegQueryValueExA(hKey, LPCSTR(keyName), 0, &dwType, (LPBYTE) output, &dwSize) != ERROR_SUCCESS) throw std::invalid_argument("Could not get value");
	//
	//// Close key
	//RegCloseKey(hKey);
	
	

	
#elif defined(__APPLE_CC__)
	CFStringRef keyString = CFStringCreateWithCString(kCFAllocatorDefault, keyName, kCFStringEncodingASCII);
	
	CFDataRef result = (CFDataRef) CFPreferencesCopyAppValue(keyString, kCFPreferencesCurrentApplication);
	if (result == NULL)
	{
		CFRelease(keyString);
		return NULL;
	}
	
	outDataSize = CFDataGetLength(result);
	UInt8 *value = new UInt8[outDataSize];
	CFDataGetBytes(result, CFRangeMake(0,CFDataGetLength(result)), value);
	
	CFRelease(keyString);
	CFRelease(result);
	
	return value;
	
#else
	// TODO: Find a way to do this on android
	return NULL;
#endif
}
