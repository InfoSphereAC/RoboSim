//
//  AppDelegate.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 31.04.10
//  Copyright 2010 RWTH Aachen University All rights reserved.
//

#import <UIKit/UIKit.h>

@class EAGLView;

@interface AppDelegate : NSObject <UIApplicationDelegate> {
	IBOutlet UIWindow *window;
	IBOutlet EAGLView *glView;
}

@property (nonatomic, retain) UIWindow *window;
@property (nonatomic, retain) EAGLView *glView;

@end


