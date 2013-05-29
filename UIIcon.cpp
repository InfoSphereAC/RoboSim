/*
 *  UIIcon.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 24.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIIcon.h"

#include <algorithm>
#include <limits.h>

#include "OpenGL.h"

UIIcon::UIIcon(float sx, float sy, float sw, float sh, float tx, float ty, float tw, float th, UIIconDragTarget *dragTarget)
: UIView(sx, sy, sw, sh), texX(tx), texY(ty), texWidth(tw), texHeight(th), target(dragTarget)
{
	setOrigin(screenX, screenY);
	
	color[0] = 1.0f;
	color[1] = 1.0f;
	color[2] = 1.0f;
	
	firstTouchID = UINT_MAX;
}

void UIIcon::draw()
{
	if (!isVisible) return;
	
	glColor4f(color[0], color[1], color[2], 1.0f);
	glVertexPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].posX));
	glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].texS));
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void UIIcon::setColor(const float *newColor)
{
	memcpy(color, newColor, sizeof(color));
}

bool UIIcon::mouseOnIcon(float x, float y)
{
	x -= screenX;
	if (x < 0.0f || x > screenWidth) return false;
	
	y -= screenY;
	if (y < 0.0f || y > screenHeight) return false;
	
	return true;
}

bool UIIcon::mouseDown(unsigned touchID, float x, float y)
{	
	if (mouseOnIcon(x, y))
	{
		touchesOnIcon.push_back(touchID);
		if (firstTouchID == UINT_MAX)
		{
			firstTouchID = touchID;
			firstTouchX = x;
			firstTouchY = y;
		}
		return true;
	}
	else
		return false;
}

bool UIIcon::mouseDragged(unsigned touchID, float x, float y)
{
	if (touchID == firstTouchID)
	{
		float deltaX = x-firstTouchX;
		float deltaY = y-firstTouchY;
		
		firstTouchX = x;
		firstTouchY = y;
		
		if (target)
			target->iconDragged(this, deltaX, deltaY);

		return true;
	}
	return std::find(touchesOnIcon.begin(), touchesOnIcon.end(), touchID) != touchesOnIcon.end();
}

bool UIIcon::mouseUp(unsigned touchID, float x, float y)
{
	std::vector<unsigned>::iterator touch = std::find(touchesOnIcon.begin(), touchesOnIcon.end(), touchID);
	if (touch == touchesOnIcon.end()) return false;
	touchesOnIcon.erase(touch);
	
	if (touchID == firstTouchID)
		firstTouchID = UINT_MAX;
	
	return true;
}

void UIIcon::changeStoredPointerCoordinates(float deltaX, float deltaY)
{
	firstTouchX += deltaX;
	firstTouchY += deltaY;
}

void UIIcon::setOrigin(float newX, float newY)
{	
	UIView::setOrigin(newX, newY);
	
	buffer[0] = VertexArrayElement(screenX + screenWidth, screenY + screenHeight, texX + texWidth, texY + texHeight);
	buffer[1] = VertexArrayElement(screenX + screenWidth, screenY, texX + texWidth, texY);
	buffer[2] = VertexArrayElement(screenX, screenY + screenHeight, texX, texY + texHeight);
	buffer[3] = VertexArrayElement(screenX, screenY, texX, texY);
}
