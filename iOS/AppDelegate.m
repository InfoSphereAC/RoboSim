//
//  AppDelegate.m
//  mindstormssimulation
//
//  Created by Torsten Kammer on 31.04.10.
//  Copyright 2010 RWTH Aachen University All rights reserved.
//

#import "AppDelegate.h"

#import "EAGLView.h"

@implementation AppDelegate

@synthesize window;
@synthesize glView;

- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	[glView startAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
	[glView setAnimationPaused:YES];
}
- (void)applicationDidBecomeActive:(UIApplication *)application
{
	[glView setAnimationPaused:NO];
}

- (void)applicationWillResignActive:(UIApplication *)application
{
	[glView setAnimationPaused:YES];
}

- (void)dealloc
{
	[window release];
	[glView release];
	[super dealloc];
}

@end
