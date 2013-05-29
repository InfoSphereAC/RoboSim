//
//  iphonemain.m
//  mindstormssimulation
//
//  Created by Torsten Kammer on 31.04.10
//  Copyright 2010 RWTH Aachen University All rights reserved.
//

#import <UIKit/UIKit.h>

int main(int argc, char *argv[]) {
	
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int retVal = UIApplicationMain(argc, argv, nil, nil);
	[pool release];
	return retVal;
}
