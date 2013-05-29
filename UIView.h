/*
 *  UIView.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <map>
#include <vector>

#pragma once

class UIView
{
protected:
	float screenX;
	float screenY;
	float screenWidth;
	float screenHeight;
	
	bool isVisible;
	
	struct VertexArrayElement
	{
		float posX;
		float posY;
		float texS;
		float texT;
		
		VertexArrayElement() {}
		VertexArrayElement(float x, float y, float s, float t) : posX(x), posY(y), texS(s), texT(t) {}
	};
	
	UIView(float x, float y, float width, float height);
	
	// Some user interface elements store pointer coordinates, which are always relative to their base coordinates. When the element's container is moved, this will stop working, so this method is needed.
public:
    virtual ~UIView() {}
	
	virtual bool mouseDown(unsigned touchID, float x, float y) = 0;
	virtual bool mouseUp(unsigned touchID, float x, float y) = 0;
	virtual bool mouseDragged(unsigned touchID, float x, float y) = 0;
	
	virtual void draw() = 0;
	
	virtual void setOrigin(float newX, float newY);
	virtual void changeStoredPointerCoordinates(float deltaX, float deltaY) {}
	
	float getWidth() const { return screenWidth; }
	float getHeight() const { return screenHeight; }
	
	bool getIsVisible() const { return isVisible; }
	virtual void setIsVisible(bool isit) { isVisible = isit; }
};

class UIContainer : public UIView
{
protected:
	std::vector<UIView *>subviews;
	std::map<unsigned, UIView *> *viewForTouch;
	
public:
	UIContainer(float x, float y, float width, float height);
	virtual ~UIContainer();
	
	virtual bool mouseDown(unsigned touchID, float x, float y);
	virtual bool mouseUp(unsigned touchID, float x, float y);
	virtual bool mouseDragged(unsigned touchID, float x, float y);
	
	virtual void changeStoredPointerCoordinates(float deltaX, float deltaY);
	
	virtual void draw();
};
