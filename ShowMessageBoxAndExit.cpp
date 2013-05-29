/*
 *  ShowMessageBoxAndExit.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ShowMessageBoxAndExit.h"

#ifdef __APPLE_CC__
#include <CoreFoundation/CoreFoundation.h>

#elif defined(ANDROID_NDK)
#include <android/log.h>

#elif defined(_WIN32)
#include <windows.h>

#endif /* Platform */

#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static void ShowErrorVa(const wchar_t *de, const wchar_t *en, va_list args)
{
	// Get current language
	const wchar_t *stringToUse = en;
#ifdef _WIN32
	if ((GetUserDefaultLangID() & 0xFF) == LANG_GERMAN) stringToUse = de;		
#elif defined(__APPLE_CC__)
	CFStringRef localisations[2] = { CFSTR("en"), CFSTR("de") };
	CFArrayRef allLocalisations = CFArrayCreate(kCFAllocatorDefault, (const void **) localisations, 2, &kCFTypeArrayCallBacks);
	CFArrayRef preferredLocalisations = CFBundleCopyPreferredLocalizationsFromArray(allLocalisations);
	CFStringRef oldStyleLang = (CFStringRef) CFArrayGetValueAtIndex(preferredLocalisations, 0);
	CFStringRef newStyleLang = CFLocaleCreateCanonicalLanguageIdentifierFromString(kCFAllocatorDefault, oldStyleLang);
	
	if (CFStringHasPrefix(newStyleLang, CFSTR("de"))) stringToUse = de;
	
	CFRelease(allLocalisations);
	CFRelease(preferredLocalisations);
	CFRelease(newStyleLang);
#else
	// TODO: Implement something here for android.
#endif
	// Create output string
	wchar_t text[1024];
	vswprintf(text, 1024, stringToUse, args);
	
	// Show message
#ifdef _WIN32
	MessageBoxW(NULL, text, L"Mindstorms Simulator", MB_TASKMODAL & MB_ICONWARNING);
#elif defined(IPHONE)
	// Ignore it
#elif defined(__APPLE_CC__)
	CFStringRef messageString = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8 *) text, sizeof(wchar_t) * wcslen(text), (CFByteOrderGetCurrent() == CFByteOrderBigEndian) ? kCFStringEncodingUTF32BE : kCFStringEncodingUTF32LE, false);
	CFUserNotificationDisplayAlert(0.0, kCFUserNotificationStopAlertLevel, NULL, NULL, NULL, messageString, NULL, NULL, NULL, NULL, NULL);
	CFRelease(messageString);
#else
	// TODO: Implement something here for android.
	size_t length = wcslen(text)+1;
	char *englishMangledASCII = new char[length];
	for (unsigned i = 0; i < length; i++)
		englishMangledASCII[i] = (char) en[i];
	
	__android_log_print(ANDROID_LOG_FATAL, "librobosim.so", "Error: %s", englishMangledASCII);
	delete englishMangledASCII;
#endif	
}

void ShowError(const wchar_t *de, const wchar_t *en, ...)
{
	va_list args;
	va_start(args, en);
	ShowErrorVa(de, en, args);
	va_end(args);
}

void ShowErrorAndExit(const wchar_t *de, const wchar_t *en, ...)
{
	va_list args;
	va_start(args, en);
	ShowErrorVa(de, en, args);
	va_end(args);
	exit(-1);
}
