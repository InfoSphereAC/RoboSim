/*
 *  UIIcon.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 24.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef UI_ICON_H
#define UI_ICON_H

#include <vector>

#include "UIView.h"

class UIIcon;

class UIIconDragTarget
{
public:
	virtual void iconDragged(UIIcon *icon, float deltaX, float deltaY) = 0;
};

class UIIcon : public UIView
{
	float texX;
	float texY;
	float texWidth;
	float texHeight;
	float color[3];
	
	VertexArrayElement buffer[4];
	
	std::vector<unsigned> touchesOnIcon;
	unsigned firstTouchID;
	float firstTouchX;
	float firstTouchY;
	UIIconDragTarget *target;
	
	bool mouseOnIcon(float x, float y);
	
public:
	UIIcon(float sx, float sy, float sw, float sh, float tx, float ty, float tw, float th, UIIconDragTarget *dragTarget = NULL);
	
	void setColor(const float *color);
	
	virtual void draw();
	virtual bool mouseDown(unsigned touchID, float x, float y);
	virtual bool mouseUp(unsigned touchID, float x, float y);
	virtual bool mouseDragged(unsigned touchID, float x, float y);
	virtual void setOrigin(float newX, float newY);
	virtual void changeStoredPointerCoordinates(float deltaX, float deltaY);
};

#endif /* UI_ICON_H */
