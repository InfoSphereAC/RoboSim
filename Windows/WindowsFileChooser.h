//
//  WindowsFileChooser.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 06.07.12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "../FileChooser.h"

class WindowsFileChooser : public FileChooser
{
	virtual void run(FileChooserDelegate *delegate);
};