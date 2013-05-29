//
//  TouchesRecognizer.cpp
//  mindstormssimulation
//
//  Created by Torsten Kammer on 27.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include "TouchesRecognizer.h"

#include <cmath>

#include "Controller.h"

namespace
{
	/*
	 * When a pointer (usually finger) touches down, it can be as part of
	 * a tap (i.e. click), as part of a motion event (i.e. scroll) or as
	 * the first of a double-tap (double-click).
	 *
	 * If it moves more than vMaxForTap pixels per second before timeForTap
	 * is up, it counts as a motion event. If it gets lifted before
	 * timeForTap is up, it gets counted as the first of a possible
	 * double-tap. If it does not come down again before gapForDoubleTap,
	 * a fake down-and-release sequence is issued instead. Finally, if it
	 * stays there until timeForTap is up (or moves very little, as
	 * determined with vMaxForTap), it is counted as a normal tap, and
	 * further moves are counted as the equivalent of mouse dragging.
	 */
	const float timeForTap = 0.2f;
	const float gapForDoubletap = 0.2f;
	const float vMaxForTap = 50.0f;
	const float timeToReportNoMotion = 0.05f;
}

TouchesRecognizer::TouchesRecognizer(Controller *aController)
: controller(aController)
{
}

TouchesRecognizer::~TouchesRecognizer()
{
	for (std::map<unsigned, SingleTouchRecognizer *>::iterator iter = activeTouches.begin(); iter != activeTouches.end(); iter++)
	{
		iter->second->cancel();
		delete iter->second;
	}
}

void TouchesRecognizer::touchDown(unsigned arbitraryIdentifier, float x, float y)
{
	std::map<unsigned, SingleTouchRecognizer *>::iterator existingTouch = activeTouches.find(arbitraryIdentifier);
	
	if (existingTouch == activeTouches.end() || !(existingTouch->second->stillValid()))
	{
		activeTouches[arbitraryIdentifier] = new SingleTouchRecognizer(controller, arbitraryIdentifier, x, y);
	}
	else
		existingTouch->second->downAgain();
}
void TouchesRecognizer::touchMoved(unsigned arbitraryIdentifier, float x, float y)
{
	std::map<unsigned, SingleTouchRecognizer *>::iterator existingTouch = activeTouches.find(arbitraryIdentifier);
	
	if (existingTouch == activeTouches.end()) return;
	
	existingTouch->second->move(x, y);
}
void TouchesRecognizer::touchUp(unsigned arbitraryIdentifier)
{
	std::map<unsigned, SingleTouchRecognizer *>::iterator existingTouch = activeTouches.find(arbitraryIdentifier);
	
	if (existingTouch == activeTouches.end()) return;
	
	existingTouch->second->up();
	
	if (!(existingTouch->second->stillValid()))
		activeTouches.erase(existingTouch);
}
void TouchesRecognizer::touchCancelled(unsigned arbitraryIdentifier)
{
	std::map<unsigned, SingleTouchRecognizer *>::iterator existingTouch = activeTouches.find(arbitraryIdentifier);
	
	if (existingTouch == activeTouches.end()) return;
	
	existingTouch->second->cancel();
	
	if (!(existingTouch->second->stillValid()))
		activeTouches.erase(existingTouch);
}
void TouchesRecognizer::update(float timedelta)
{
	for (std::map<unsigned, SingleTouchRecognizer *>::iterator iter = activeTouches.begin(); iter != activeTouches.end(); iter++)
	{
		iter->second->update(timedelta);
	}
}

SingleTouchRecognizer::SingleTouchRecognizer(Controller *aController, unsigned anIdentifier, float x, float y)
: controller(aController),
identifier(anIdentifier),
timeSinceLastEvent(0.0f),
initialX(x),
initialY(y),
lastX(x),
lastY(y),
type(Unclear)
{
}

void SingleTouchRecognizer::update(float timeDelta)
{
	timeSinceLastEvent += timeDelta;
	
	if (type == Unclear)
	{
		if (timeSinceLastEvent < timeForTap) return;
		
		type = Tap;
		
		controller->tapDown(identifier, lastX, lastY);
	}
	else if (type == PossibleDoubletap)
	{
		if (timeSinceLastEvent > gapForDoubletap)
		{
			// Count as normal tap up and down.
			type = Ended;
			controller->tapDown(identifier, lastX, lastY);
			controller->tapUp(identifier, lastX, lastY);
		}
	}
	else if (type == Motion)
	{
		if (timeSinceLastEvent > timeToReportNoMotion)
			controller->motion(identifier, lastX, lastY, lastX, lastY);
	}
}

void SingleTouchRecognizer::move(float x, float y)
{	
	float dist = std::sqrt((lastX-x)*(lastX-x)+(lastY-y)*(lastY-y));
	if ((dist/timeSinceLastEvent > vMaxForTap) && (type == Unclear))
		type = Motion;
	
	if (type == Motion)
	{
		controller->motion(identifier, lastX, lastY, x, y);
	}
	else if (type == Tap)
	{
		controller->tapDragged(identifier, x, y);
	}
	timeSinceLastEvent = 0.0f;
	
	lastX = x;
	lastY = y;
}

void SingleTouchRecognizer::up()
{
	if (type == Unclear)
	{
		type = PossibleDoubletap;
		timeSinceLastEvent = 0.0f;
		return;
	}
	else
	{
		if (type == Tap)
			controller->tapUp(identifier, lastX, lastY);
		else if (type == Motion)
			controller->motionEnded(identifier);
		type = Ended;
	}
}

void SingleTouchRecognizer::downAgain()
{
	if (type != PossibleDoubletap)
		return;
	
	controller->doubletap(lastX, lastY);
	type = Ended;
}

bool SingleTouchRecognizer::stillValid()
{
	return type != Ended;
}

void SingleTouchRecognizer::cancel()
{
	up();
	type = Ended;
}
