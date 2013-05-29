//
//  RobosimView.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>

class Controller;

@interface RobosimView : NSOpenGLView
{
	Controller *controller;
	CVDisplayLinkRef displayLink;
	NSTimer *updateTimer;
	NSTimeInterval oldTime;
	
	NSString *fileToOpen;
	
	BOOL openGLLoaded;
}

@end
