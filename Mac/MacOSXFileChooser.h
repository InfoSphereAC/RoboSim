//
//  MacOSXFileChooser.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "../FileChooser.h"

class MacOSXFileChooser : public FileChooser
{
public:
	
	virtual void run(FileChooserDelegate *delegate);
};
