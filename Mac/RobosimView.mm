//
//  RobosimView.m
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RobosimView.h"

#import "Controller.h"

static int argCount;
static const char **argValues;

static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
									const CVTimeStamp *inNow,
									const CVTimeStamp *inOutputTime,
									CVOptionFlags flagsIn,
									CVOptionFlags *flagsOut,
									void *displayLinkContext)
{
	[(RobosimView *) displayLinkContext drawRect:NSZeroRect];
	
	return kCVReturnSuccess;
}

@implementation RobosimView

- (void)prepareOpenGL
{
	if (openGLLoaded) return;
	if (!controller) return;
	openGLLoaded = YES;
	
	controller->initVideo();
	
	CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
	CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink, (CGLContextObj) [[self openGLContext] CGLContextObj], (CGLPixelFormatObj) [[self pixelFormat] CGLPixelFormatObj]);
	CVDisplayLinkSetOutputCallback(displayLink, DisplayLinkCallback, self);
	CVDisplayLinkStart(displayLink);
}

- (void)reshape
{
	if (!controller) return;
	
	NSRect bounds = [self bounds];

	controller->setWindowSize(bounds.size.width, bounds.size.height);
	controller->setScreenOrientation(Controller::Normal);
}

- (void)drawRect:(NSRect)rect
{
	if (!controller) return;
	
	[[self openGLContext] makeCurrentContext];
	controller->draw();
	[[self openGLContext] flushBuffer];
}

- (void)update:(id)timer
{
	NSTimeInterval now = [NSDate timeIntervalSinceReferenceDate];
	NSTimeInterval delta = now - oldTime;
	oldTime = now;
	
	try
	{
		controller->update(delta);	
	}
	catch (std::exception &e)
	{
		NSLog(@"Error in run loop: %s", e.what());
	}
}

#pragma mark -

- (BOOL)acceptsFirstResponder
{
	return YES;
}

- (void)keyDown:(NSEvent *)theEvent
{
	NSString *chars = [theEvent charactersIgnoringModifiers];
	int character = (int) [chars characterAtIndex:0];
	bool isPrintable = true;
	
	if (character == NSUpArrowFunctionKey)
	{
		character = Controller::UpArrow;
		isPrintable = false;
	}
	else if (character == NSDownArrowFunctionKey)
	{
		character = Controller::DownArrow;
		isPrintable = false;
	}
	else if (character == NSLeftArrowFunctionKey)
	{
		character = Controller::LeftArrow;
		isPrintable = false;
	}
	else if (character == NSRightArrowFunctionKey)
	{
		character = Controller::RightArrow;
		isPrintable = false;
	}
	
	controller->keyDown(character, isPrintable);
}

- (void)keyUp:(NSEvent *)theEvent
{
	NSString *chars = [theEvent charactersIgnoringModifiers];
	int character = (int) [chars characterAtIndex:0];
	bool isPrintable = true;
	
	if (character == NSUpArrowFunctionKey)
	{
		character = Controller::UpArrow;
		isPrintable = false;
	}
	else if (character == NSDownArrowFunctionKey)
	{
		character = Controller::DownArrow;
		isPrintable = false;
	}
	else if (character == NSLeftArrowFunctionKey)
	{
		character = Controller::LeftArrow;
		isPrintable = false;
	}
	else if (character == NSRightArrowFunctionKey)
	{
		character = Controller::RightArrow;
		isPrintable = false;
	}	
	
	controller->keyUp(character, isPrintable);
}

- (void)mouseDown:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseDown(point.x, point.y, 0);
}
- (void)mouseUp:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseUp(point.x, point.y, 0);
}
- (void)mouseDragged:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseMove(point.x, point.y);
}

- (void)rightMouseDown:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseDown(point.x, point.y, 1);
}
- (void)rightMouseUp:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseUp(point.x, point.y, 1);
}
- (void)rightMouseDragged:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseMove(point.x, point.y);
}

- (void)otherMouseDown:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseDown(point.x, point.y, [theEvent buttonNumber]);
}
- (void)otherMouseUp:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseUp(point.x, point.y, [theEvent buttonNumber]);
}
- (void)otherMouseDragged:(NSEvent *)theEvent;
{
	NSPoint point = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	controller->mouseMove(point.x, point.y);
}

- (void)scrollWheel:(NSEvent *)theEvent
{
	controller->scroll([theEvent deltaY]);
}

#pragma mark -

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
	return YES;
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
{
	fileToOpen = [filename copy];
	return YES;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	char resourcedirectory[1024];
	CFURLRef url = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
	if (CFURLGetFileSystemRepresentation(url, 1, (UInt8 *)resourcedirectory, 1024))
	{
		chdir(resourcedirectory);   /* chdir to the binary app's resources directory */
	}
	CFRelease(url);
	
	if (argCount == 1)
	{
		controller = new Controller(Controller::LetUserChoose, 0, [fileToOpen fileSystemRepresentation], NULL, NULL);
	}
	else
	{
		// Parse arguments
		
		const char *filename = NULL;
		char server[255];
		server[0] = 0;
		char port[255]; // Yes, as string, because thatís what parsing functions later use.
		port[0] = 0;
		bool robotUIOnServer = false;
		bool noAutoDiscovery = false;
		
		Controller::NetworkMode mode = Controller::LetUserChoose;
		unsigned flags = 0;
		
		for (int i = 1; i < argCount; i++)
		{
			char serverTempString[255];
			if (strcmp(argValues[i], "--help") == 0 || strcmp(argValues[i], "-h") == 0 || strcmp(argValues[i], "-?") == 0)
			{
				printf("Usage: Normally from the GUI, but if not\n\t%s [args] [filename]\nOptional arguments:", argValues[0]);
				printf("\n\t-s=,--server=\tServer to connect to (bypasses selection screen)");
				printf("\n\t--asServer Run as server");
				printf("\n\t-p=,--port=\tPort (default 10412) to connect to/of server");
				printf("\n\t--robotUIOnServer Only valid for clients: Stuff like picking up and IO configuration is handled by server.\nOnly one such robot is allowed per server, and only if the server does not have a robot of its own.");	
				printf("\n\t--noAutodiscovery Only valid for server: Do not respond to autodiscovery messages. This means clients have to know the port and IP address of a server to connect to it.");
				printf("\n\t--\tStop scanning for arguments.");
			}
			else if (sscanf(argValues[i], "--server=%255s", serverTempString) == 1 || sscanf(argValues[i], "-s=%255s", serverTempString) == 1)
			{
				memcpy(server, serverTempString, 255);
				mode = Controller::ClientMode;
			}
			else if (sscanf(argValues[i], "--port=%255s", serverTempString) == 1 || sscanf(argValues[i], "-p=%255s", serverTempString) == 1)
				memcpy(port, serverTempString, 255);
			else if (strcmp(argValues[i], "--asServer") == 0)
				mode = Controller::ServerMode;
			else if (strcmp(argValues[i], "--noAutodiscovery") == 0)
				noAutoDiscovery = true;
			else if (strcmp(argValues[i], "--robotUIOnServer") == 0)
				robotUIOnServer = true;
			else if (strcmp(argValues[i], "-NSDocumentRevisionsDebugMode") == 0)
				i++; // Skip next.
			else if (strcmp(argValues[i], "--") == 0)
			{
				if (argCount > (i+1)) filename = argValues[i+1];
				break;
			}
			else
				filename = argValues[i];
		}
		
		server[254] = 0;
		port[254] = 0;
		
		if (mode == Controller::ClientMode && robotUIOnServer)
			flags |= Controller::ClientFlagUIOnServer;
		else if (mode == Controller::ServerMode && noAutoDiscovery)
			flags |= Controller::ServerFlagNoBroadcast;
		
		controller = new Controller(mode, flags, filename, server[0] ? server : NULL, port[0] ? port : NULL);
	}
	
	[self prepareOpenGL];
	[self reshape];
	
	controller->initAudio();
	
	updateTimer = [NSTimer scheduledTimerWithTimeInterval:1.0f/60.0f target:self selector:@selector(update:) userInfo:nil repeats:YES];
	oldTime = [NSDate timeIntervalSinceReferenceDate];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	controller->shutDown();
}

@end

extern "C" int main (int argc, const char * argv[])
{
	if ( argc >= 2 && strncmp (argv[1], "-psn", 4) == 0 )
	{
		// Launched from Finder
		argCount = 1;
		argValues = (const char **) malloc(sizeof(char *) * 2);
		argValues[0] = argv[0];
		argValues[1] = NULL;
    }
	else
	{
		argCount = argc;
		argValues = (const char **) malloc(sizeof(char *) * (argCount + 1));
		for (int i = 0; i < argc; i++)
			argValues[i] = argv[i];
    }
	
	NSApplicationMain(argc, argv);
}
