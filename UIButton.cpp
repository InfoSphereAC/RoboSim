/*
 *  UIButton.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIButton.h"

#include <limits.h>

#include "OpenGL.h"

UIButton::UIButton(UIButtonTarget *aTarget, float x, float y, float width, float height)
: UIView(x, y, width, height), target(aTarget)
{
	currentState = Normal;
	hasState = 0;
	touchOnButton = UINT_MAX;
}

// Graphics
void UIButton::setTextureRectForState(State aState, float x, float y, float width, float height)
{
	texture[aState].x = x;
	texture[aState].y = y;
	texture[aState].width = width;
	texture[aState].height = height;
	
	hasState |= 1 << aState;
	
	// Reload graphics if necessary.
	if (aState == currentState) setState(currentState);
}

void UIButton::setState(State aState)
{
	if (!(hasState & (1 << aState))) return;
	
	currentState = aState;
	
	buffer[0] = VertexArrayElement(screenX + screenWidth, screenY + screenHeight, texture[aState].x + texture[aState].width, texture[aState].y + texture[aState].height);
	buffer[1] = VertexArrayElement(screenX + screenWidth, screenY, texture[aState].x + texture[aState].width, texture[aState].y);
	buffer[2] = VertexArrayElement(screenX, screenY + screenHeight, texture[aState].x, texture[aState].y + texture[aState].height);
	
	buffer[3] = VertexArrayElement(screenX, screenY, texture[aState].x, texture[aState].y);
}

void UIButton::setOrigin(float newX, float newY)
{
	UIView::setOrigin(newX, newY);
	setState(currentState);
}

void UIButton::draw()
{
	if (!isVisible) return;
	
	glVertexPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].posX));
	glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].texS));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

// Mouse handling

bool UIButton::mouseOnButton(float x, float y)
{
	x -= screenX;
	if (x < 0.0f || x > screenWidth) return false;
	
	y -= screenY;
	if (y < 0.0f || y > screenHeight) return false;
	
	return true;
}

bool UIButton::mouseDown(unsigned touchID, float x, float y)
{	
	if (touchOnButton != UINT_MAX) return false;
	if (!mouseOnButton(x, y)) return false;
	touchOnButton = touchID;
	
	// Switch to active state
	if (currentState == Normal) setState(NormalActive);
	else if (currentState == Secondary) setState(SecondaryActive);
	
	return true;
}

bool UIButton::mouseDragged(unsigned touchID, float x, float y)
{
	if (touchID != touchOnButton) return false;
	
	if (mouseOnButton(x, y))
	{
		if (currentState == Normal) setState(NormalActive);
		else if (currentState == Secondary) setState(SecondaryActive);
		return true;
	}
	else
	{
		if (currentState == NormalActive) setState(Normal);
		else if (currentState == SecondaryActive) setState(Secondary);
		return false;
	}
}

bool UIButton::mouseUp(unsigned touchID, float x, float y)
{
	if (touchID == touchOnButton)
		touchOnButton = UINT_MAX;
	else return false;
	if (!mouseOnButton(x, y)) return false;
	
	// Switch to inactive state
	if (currentState == NormalActive) setState(Normal);
	else if (currentState == SecondaryActive) setState(Secondary);
	
	// Call action
	target->action(this);
	
	return true;
}

UICircularButton::UICircularButton(UIButtonTarget *target, float x, float y, float width, float height)
: UIButton(target, x, y, width, height)
{
}

bool UICircularButton::mouseOnButton(float x, float y)
{
	if (!UIButton::mouseOnButton(x, y)) return false;
	
	// Transform to relative of center of button
	x -= screenX + 0.5f*screenWidth;
	y -= screenY + 0.5f*screenHeight;
	
	// Check
	return x*x + y*y <= screenWidth*screenHeight*0.25f;
}