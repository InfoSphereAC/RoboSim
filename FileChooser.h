#ifndef FILE_CHOOSER_H
#define FILE_CHOOSER_H
//
//  FileChooser.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 12.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

class FileChooserDelegate
{
public:
	virtual void fileChooserFoundFile(const char *path) = 0;
};

class FileChooser
{
protected:
	FileChooserDelegate *delegate;
	
	FileChooser() {}
	
	static void setSharedIfNoneExists(FileChooser *chooser);
	
public:
	static FileChooser *sharedFileChooser();
	
	virtual void run(FileChooserDelegate *delegate) = 0;
};

#endif /* FILE_CHOOSER_H */