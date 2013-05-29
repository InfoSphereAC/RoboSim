//
//  RotatingViewController.m
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "RotatingViewController.h"


@implementation RotatingViewController

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		// iPad supports all orientations
		return YES;
	}
	else
	{
		// iPhone supports landscape only
		return UIInterfaceOrientationIsLandscape(interfaceOrientation);
	}

}

@end
