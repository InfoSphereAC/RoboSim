/*
 *  ShowMessageBoxAndExit.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @abstract Outputs an error description in german or english and then
 * exits the program.
 * @discussion This is based on sprintf.  The notable feature is that it
 * has two format strings, the first one for german, the second for english.
 * Both use the same arguments. The current OS setting is used to find out
 * which string to use. If neither english or german it defaults to english.
 */
void ShowErrorAndExit(const wchar_t *de, const wchar_t *en, ...);

/*!
 * @abstract Outputs an error description in german or english.
 * @discussion Same as ShowErrorAndExit, but this one does not quit the program.
 */

void ShowError(const wchar_t *de, const wchar_t *en, ...);
	
#ifdef __cplusplus
}
#endif