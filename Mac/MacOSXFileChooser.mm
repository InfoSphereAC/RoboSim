//
//  MacOSXFileChooser.mm
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "MacOSXFileChooser.h"

#import <Cocoa/Cocoa.h>

void MacOSXFileChooser::run(FileChooserDelegate *delegate)
{
	NSOpenPanel *openPanel = [NSOpenPanel openPanel];
	openPanel.allowsMultipleSelection = NO;
	
	[openPanel beginSheetModalForWindow:[[NSApplication sharedApplication] mainWindow] completionHandler:^(NSInteger returnCode) {
		
		if (returnCode != NSOKButton) return;
		
		delegate->fileChooserFoundFile(openPanel.URL.path.fileSystemRepresentation);
	}];
}
