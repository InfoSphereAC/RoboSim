//
//  RotatingViewController.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 29.05.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <UIKit/UIKit.h>

/*!
 * @abstract An UIViewController that allows rotation depending on the device.
 * @discussion The only difference is that shouldAutorotateToInterfaceOrientation: is overridden. On iPad, it always returns YES, while on iPhone, it only allows landscape mode.
 */

@interface RotatingViewController : UIViewController {
    
}

@end
