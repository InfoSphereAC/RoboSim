//
//  EAGLView.h
//  Oker Valley Railroad
//
//  Created by Torsten Kammer on 23.09.08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//


#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>

#ifdef __cplusplus
class Controller;
#else
typedef void Controller;
#endif

/*
This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
The view content is basically an EAGL surface you render your OpenGL scene into.
Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
*/
@interface EAGLView : UIView <UIGestureRecognizerDelegate> {
	
@private
	/* The pixel dimensions of the backbuffer */
	GLint backingWidth;
	GLint backingHeight;
	
	EAGLContext *context;
	
	/* OpenGL names for the renderbuffer and framebuffers used to render to this view */
	GLuint viewRenderbuffer, viewFramebuffer;
	
	/* OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist) */
	GLuint depthRenderbuffer;
	
	Controller *controller;
	NSTimer *updateTimer;
	NSTimeInterval oldTime;

	CADisplayLink *displayLink;
}

- (void)drawViewWithDisplayLink:(CADisplayLink *)link;

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event;


- (void)startAnimation;
- (void)setAnimationPaused:(BOOL)isPaused;

@end
