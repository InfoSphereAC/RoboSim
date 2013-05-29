//
//  iOSFileChooser.mm
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "iOSFileChooser.h"
#import "FileChooserViewController.h"

iOSFileChooser::iOSFileChooser()
{
	chooserController = [[FileChooserViewController alloc] initWithStyle:UITableViewStylePlain];
	
	navController = [[UINavigationController alloc] initWithRootViewController:(id) chooserController];
}

iOSFileChooser::~iOSFileChooser()
{
	[(id) chooserController release];
	[(id) navController release];
}

void iOSFileChooser::run(FileChooserDelegate *delegate)
{
	FileChooserViewController *controller = (FileChooserViewController *) chooserController;
	controller.delegate = delegate;
	UIWindow *window = [[UIApplication sharedApplication] keyWindow];
	
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		// iPad
		
		UIView *view = window.rootViewController.view;
		
		if (!controller.popoverController)
		{
			UIPopoverController *popoverController = [[UIPopoverController alloc] initWithContentViewController:(id) navController];
			popoverController.passthroughViews = [NSArray arrayWithObject:view];
			controller.popoverController = popoverController;
		}
		
		CGRect button = CGRectMake(150.0f, 10.0f, 64.0f, 64.0f);
		
		[controller.popoverController presentPopoverFromRect:button inView:view permittedArrowDirections:UIPopoverArrowDirectionAny animated:YES];
	}
	else
	{
		// iPhone
		[window.rootViewController presentModalViewController:(id) navController animated:YES];
	}
}
