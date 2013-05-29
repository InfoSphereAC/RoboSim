//
//  EAGLView.m
//  Oker Valley Railroad
//
//  Created by Torsten Kammer on 23.09.08.
//  Copyright __MyCompanyName__ 2008. All rights reserved.
//



#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#import "EAGLView.h"

#include "Controller.h"
#include "OpenGL.h"

// A class extension to declare private methods
@interface EAGLView ()

- (BOOL) createFramebuffer;
- (void) destroyFramebuffer;

@end

@implementation EAGLView

// You must implement this
+ (Class)layerClass
{
	return [CAEAGLLayer class];
}


//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id)initWithCoder:(NSCoder*)coder {

	if ((self = [super initWithCoder:coder])) {
		// Get the layer
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
		
		eaglLayer.opaque = YES;
		eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
		   [NSNumber numberWithBool:NO], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		
		if (!context || ![EAGLContext setCurrentContext:context]) {
			[self release];
			return nil;
		}

		self.multipleTouchEnabled = YES;
		
		displayLink = [[CADisplayLink displayLinkWithTarget:self selector:@selector(drawViewWithDisplayLink:)] retain];
				
		[[NSFileManager defaultManager] changeCurrentDirectoryPath:[[NSBundle mainBundle] resourcePath]];
		
		[EAGLContext setCurrentContext:context];
		[self createFramebuffer];
		controller = new Controller(Controller::LetUserChoose, 0, NULL, NULL, NULL);	checkGLError();
		controller->initVideo();
		controller->setWindowSize(backingWidth, backingHeight);
		controller->initAudio();
	}
	return self;
}

- (void)drawViewWithDisplayLink:(CADisplayLink *)aLink
{
	if ([[UIApplication sharedApplication] applicationState] == UIApplicationStateBackground) return;

	[EAGLContext setCurrentContext:context];

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	
	controller->draw();
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context presentRenderbuffer:GL_RENDERBUFFER_OES]; 
}


- (void)layoutSubviews
{
	if ([[UIApplication sharedApplication] applicationState] == UIApplicationStateBackground) return;
	
	[EAGLContext setCurrentContext:context];
	[self destroyFramebuffer];
	[self createFramebuffer];
	controller->setWindowSize(backingWidth, backingHeight);
	[self drawViewWithDisplayLink:nil];
}


- (BOOL)createFramebuffer
{
	checkGLError();
	glGenFramebuffersOES(1, &viewFramebuffer);
	glGenRenderbuffersOES(1, &viewRenderbuffer);
	glGenRenderbuffersOES(1, &depthRenderbuffer);
	
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);
	
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, backingWidth, backingHeight);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);

	if(glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES) {
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
		return NO;
	}
	checkGLError();
	
	return YES;
}


- (void)destroyFramebuffer
{	
	glDeleteFramebuffersOES(1, &viewFramebuffer);
	viewFramebuffer = 0;
	glDeleteRenderbuffersOES(1, &viewRenderbuffer);
	viewRenderbuffer = 0;
	glDeleteRenderbuffersOES(1, &depthRenderbuffer);
	depthRenderbuffer = 0;
	checkGLError();
}

- (void)startAnimation
{
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	
	updateTimer = [NSTimer scheduledTimerWithTimeInterval:1.0f/60.0f target:self selector:@selector(update:) userInfo:nil repeats:YES];
	oldTime = [NSDate timeIntervalSinceReferenceDate];
}

- (void)setAnimationPaused:(BOOL)isPaused
{
	displayLink.paused = isPaused;
	controller->setIsPaused((bool) isPaused);
	if (isPaused)
	{
		[updateTimer invalidate];
		updateTimer = nil;
	}
	else if (!updateTimer)
	{
		updateTimer = [NSTimer scheduledTimerWithTimeInterval:1.0f/60.0f target:self selector:@selector(update:) userInfo:nil repeats:YES];
	}
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

- (void)dealloc
{
	[displayLink removeFromRunLoop:[NSRunLoop mainRunLoop] forMode:NSDefaultRunLoopMode];
	[displayLink release];
	
	if ([EAGLContext currentContext] == context) {
		[EAGLContext setCurrentContext:nil];
	}
		
	[context release];	
	[super dealloc];
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
	{
		CGPoint location = [touch locationInView:self];
		controller->touchDown(reinterpret_cast<unsigned>(touch), location.x, location.y);
	}
}
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
	{
		CGPoint location = [touch locationInView:self];
		controller->touchMoved(reinterpret_cast<unsigned>(touch), location.x, location.y);
	}

}
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
		controller->touchUp(reinterpret_cast<unsigned>(touch));
}
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
		controller->touchCancelled(reinterpret_cast<unsigned>(touch));
}

@end
