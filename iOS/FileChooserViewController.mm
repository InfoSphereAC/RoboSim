//
//  FileChooserViewController.m
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FileChooserViewController.h"

#import "iOSFileChooser.h"

@implementation FileChooserViewController

@synthesize delegate;
@synthesize popoverController;

#pragma mark -
#pragma mark View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];

	UIBarButtonItem *item = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel target:self action:@selector(cancel:)];
	self.navigationItem.rightBarButtonItem = item;
	self.title = NSLocalizedString(@"Select a file", "File chooser view controller title");
		
	files = [[NSMutableArray alloc] init];
}

- (void)viewWillAppear:(BOOL)animated
{
	[self reload];
    [super viewWillAppear:animated];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma mark -
#pragma mark Data handling

- (void)reload
{
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);	
	NSURL *documentsDirectory = [NSURL fileURLWithPath:[paths objectAtIndex:0]];
	
	NSFileManager *manager = [NSFileManager defaultManager];
	NSArray *properties = [NSArray arrayWithObjects:NSURLIsRegularFileKey, NSURLEffectiveIconKey, nil];
	
	NSArray *includedFiles = [manager contentsOfDirectoryAtURL:documentsDirectory includingPropertiesForKeys:properties options:NSDirectoryEnumerationSkipsHiddenFiles error:NULL];
	[files removeAllObjects];
	
	for (NSURL *file in includedFiles)
	{
		NSDictionary *resourceValues = [file resourceValuesForKeys:properties error:NULL];
		
		if (resourceValues && [[resourceValues objectForKey:NSURLIsRegularFileKey] boolValue] != YES) continue;
		if (![[file pathExtension] isEqual:@"rxe"]) continue;
		
		[files addObject:file];
	}
	
	[self.tableView reloadData];
}

#pragma mark -
#pragma mark Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
    // Return the number of sections.
    return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    // Return the number of rows in the section.
    return [files count];
}


// Customize the appearance of table view cells.
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    
    static NSString *CellIdentifier = @"Cell";
    
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
	
    if (!cell)
	{
        cell = [[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:CellIdentifier] autorelease];
    }
    
	NSURL *url = [files objectAtIndex:indexPath.row];
	UIImage *image = nil;
	[url getResourceValue:&image forKey:NSURLEffectiveIconKey error:NULL];
	cell.imageView.image = image;
	cell.textLabel.text = [url lastPathComponent];
	
    return cell;
}

#pragma mark -
#pragma mark User input

- (void)dismiss
{
	if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad)
	{
		[self.popoverController dismissPopoverAnimated:YES];
	}
	else
	{
		[self.navigationController.parentViewController dismissModalViewControllerAnimated:YES];
	}
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
	NSURL *url = [files objectAtIndex:indexPath.row];
	
	NSString *path = [url path];
	
	if (delegate)
		delegate->fileChooserFoundFile([path fileSystemRepresentation]);	
	
	[self dismiss];
}

- (void)cancel:(id)sender
{
	[self dismiss];
}

#pragma mark -
#pragma mark Memory management

- (void)dealloc
{
	[files release];
	self.delegate = NULL;
    [super dealloc];
}


@end

