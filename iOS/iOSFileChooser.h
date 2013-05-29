//
//  iOSFileChooser.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "../FileChooser.h"

class iOSFileChooser : public FileChooser
{
protected:
	void *chooserController;
	void *navController;
public:
	iOSFileChooser();
	~iOSFileChooser();
	
	virtual void run(FileChooserDelegate *delegate);
};
