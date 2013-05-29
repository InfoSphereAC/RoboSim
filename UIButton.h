/*
 *  UIButton.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "UIView.h"

class UIButton;

class UIButtonTarget
{
protected:
    virtual ~UIButtonTarget() {}
public:
	virtual void action(UIButton *sender) = 0;
};

class UIButton : public UIView
{
public:
	enum State
	{
		Normal = 0,
		NormalActive,
		Secondary,
		SecondaryActive
	};
	
protected:
	
	VertexArrayElement buffer[4];
	
	struct
	{
		float x;
		float y;
		float width;
		float height;
	} texture[4];
	
	unsigned hasState;
	
	State currentState;
	unsigned touchOnButton;
	
	virtual bool mouseOnButton(float x, float y);
	
	UIButtonTarget *target;
public:
	UIButton(UIButtonTarget *target, float x, float y, float width, float height);
    virtual ~UIButton() {}
	
	State getState() { return currentState; }
	void setState(State newState);
	
	void setTextureRectForState(State aState, float x, float y, float width, float height);
	
	virtual bool mouseDown(unsigned touchID, float x, float y);
	virtual bool mouseUp(unsigned touchID, float x, float y);
	virtual bool mouseDragged(unsigned touchID, float x, float y);
	virtual void setOrigin(float newX, float newY);
	
	void draw();
};

class UICircularButton : public UIButton
{
protected:
	virtual bool mouseOnButton(float x, float y);
public:
	UICircularButton(UIButtonTarget *target, float x, float y, float width, float height);
    virtual ~UICircularButton() {}
};
