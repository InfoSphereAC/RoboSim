//
//  TouchesRecognizer.h
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <map>

class Controller;
class SingleTouchRecognizer;

class TouchesRecognizer
{
	Controller *controller;
	std::map<unsigned, SingleTouchRecognizer *> activeTouches;
	
	void cleanup();
	
public:
	TouchesRecognizer(Controller *aController);
	~TouchesRecognizer();
	
	void update(float timedelta);
	
	void touchDown(unsigned arbitraryIdentifier, float x, float y);
	void touchMoved(unsigned arbitraryIdentifier, float x, float y);
	void touchUp(unsigned arbitraryIdentifier);
	void touchCancelled(unsigned arbitraryIdentifier);
};

class SingleTouchRecognizer
{
	Controller *controller;
	unsigned identifier;
	
	// Initial touch for unclear events, up for possible doubletap events.
	float timeSinceLastEvent;
	
	float initialX;
	float initialY;
	
	float lastX;
	float lastY;
	
	
	enum TouchType
	{
		Unclear,
		Tap,
		PossibleDoubletap,
		Motion,
		Ended
	};
	TouchType type;
	
public:
	SingleTouchRecognizer(Controller *aController, unsigned anIdentifier, float x, float y);
	
	void move(float x, float y);
	void update(float timeDelta);
	void up();
	void downAgain();
	void cancel();
	
	bool stillValid();
};
