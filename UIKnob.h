/*
 *  UIKnob.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef UI_KNOB_H
#define UI_KNOB_H

#include "UIView.h"

class UIKnob;

class UIKnobTarget
{
public:
	virtual void knobAngleChanged(UIKnob *knob, float newAngle) = 0;
};

class UIKnob : public UIView
{
	float knobX;
	float knobY;
	float knobRadius;
	
	float texX;
	float texY;
	float texWidth;
	float texHeight;
	
	float bgTexX;
	float bgTexY;
	float bgTexWidth;
	float bgTexHeight;
	
	VertexArrayElement buffer[12];
	float angle;
	
	unsigned currentTouch;
	UIKnobTarget *target;
	
	void setAngle(float angle, bool updateTarget);
public:
	UIKnob(float sx, float sy, float sw, float sh, float knobX, float knobY, float knobRadius, UIKnobTarget *target);
	
	void setKnobTexture(float tx, float ty, float tw, float th);
	void setBackgroundTexture(float tx, float ty, float tw, float th); 
	
	void setAngle(float angle);
	float getAngle() const { return angle; }
	
	virtual void draw();
	virtual bool mouseDown(unsigned touchID, float x, float y);
	virtual bool mouseUp(unsigned touchID, float x, float y);
	virtual bool mouseDragged(unsigned touchID, float x, float y);		
};

#endif /* UI_KNOB_H */
