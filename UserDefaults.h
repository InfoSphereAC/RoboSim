//
//  UserDefaults.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.11.10.
//  Copyright (c) 2010 __MyCompanyName__. All rights reserved.
//
	
void setDataForUserInterfaceKey(const char *keyName, unsigned dataSize, void *data);
void *getDataAndSizeForUserInterfaceKey(const char *keyName, unsigned &outDataSize);
