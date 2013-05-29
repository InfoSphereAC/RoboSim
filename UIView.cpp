/*
 *  UIView.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIView.h"

#include "OpenGL.h"

UIView::UIView(float x, float y, float width, float height)
: screenX(x), screenY(y), screenWidth(width), screenHeight(height), isVisible(true)
{
}

void UIView::setOrigin(float newX, float newY)
{
	changeStoredPointerCoordinates(screenX-newX, screenY-newY);
	screenX = newX;
	screenY = newY;
}

void UIContainer::changeStoredPointerCoordinates(float deltaX, float deltaY)
{
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		if (*iter == NULL) continue;
		(*iter)->changeStoredPointerCoordinates(deltaX, deltaY);
	}
}

bool UIContainer::mouseDown(unsigned touchID, float x, float y)
{
	if (!isVisible) return false;

	x -= screenX;
	y -= screenY;
	
	if (x < 0.0f || x > screenWidth) return false;
	if (y < 0.0f || y > screenHeight) return false;
	
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		if (*iter == NULL) continue;
		
		if ((*iter)->mouseDown(touchID, x, y))
		{
			(*viewForTouch)[touchID] = *iter;
			return true;
		}
	}
	
	return false;
}

bool UIContainer::mouseUp(unsigned touchID, float x, float y)
{
	if (!isVisible) return false;
	
	x -= screenX;
	y -= screenY;

	std::map<unsigned, UIView *>::iterator touchView = viewForTouch->find(touchID);

	if (touchView == viewForTouch->end() || touchView->second == NULL) return false;
	touchView->second->mouseUp(touchID, x, y);
	touchView->second = NULL;
	
	return true;
}

bool UIContainer::mouseDragged(unsigned touchID, float x, float y)
{
	if (!isVisible) return false;

	x -= screenX;
	y -= screenY;

	std::map<unsigned, UIView *>::iterator touchView = viewForTouch->find(touchID);
	
	if (touchView == viewForTouch->end() || touchView->second == NULL) return false;
	return touchView->second->mouseDragged(touchID, x, y);
}

void UIContainer::draw()
{
	if (!isVisible) return;
	
	glPushMatrix();
	glTranslatef(screenX, screenY, 0.0f);
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		if (*iter)
			(*iter)->draw();
	}
	glPopMatrix();
}

UIContainer::UIContainer(float x, float y, float width, float height)
: UIView(x, y, width, height)
{
	viewForTouch = new std::map<unsigned, UIView *>;
}

UIContainer::~UIContainer()
{
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		if (*iter)
			delete *iter;
	}
}
