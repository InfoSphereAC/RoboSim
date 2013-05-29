//
//  FileChooserViewController.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

class FileChooserDelegate;

@interface FileChooserViewController : UITableViewController
{
	NSMutableArray *files;
	FileChooserDelegate *delegate;
	UIPopoverController *popoverController;
}

- (void)reload;
- (void)dismiss;

@property (nonatomic, assign) FileChooserDelegate *delegate;
@property (nonatomic, retain) UIPopoverController *popoverController;

@end
