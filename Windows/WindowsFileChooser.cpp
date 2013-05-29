//
//  WindowsFileChooser.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 06.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "WindowsFileChooser.h"

#include <windows.h>

void WindowsFileChooser::run(FileChooserDelegate *delegate)
{
	if (!delegate) return;
	
	OPENFILENAMEA ofn;
	CHAR szFile[2600];
	
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Mindstorms Code\0*.RXE\0";
	ofn.nFilterINdex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	if (GetOpenFilenameA(&ofn) == TRUE)
	{
		delegate->FileChooserDelegate(ofn.szFile);
	}
}