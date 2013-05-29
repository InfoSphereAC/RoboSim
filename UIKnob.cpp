/*
 *  UIKnob.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIKnob.h"

#include "OpenGL.h"
#include "Vec4.h"

UIKnob::UIKnob(float sx, float sy, float sw, float sh, float kx, float ky, float kr, UIKnobTarget *aTarget)
: UIView(sx, sy, sw, sh), knobX(kx), knobY(ky), knobRadius(kr), currentTouch(UINT_MAX), target(aTarget)
{
}

void UIKnob::setKnobTexture(float tx, float ty, float tw, float th)
{
	texX = tx;
	texY = ty;
	texWidth = tw;
	texHeight = tw;
	
	setAngle(0.0f, false);
}

void UIKnob::setBackgroundTexture(float tx, float ty, float tw, float th)
{
	bgTexX = tx;
	bgTexY = ty;
	bgTexWidth = tw;
	bgTexHeight = th;
	
	buffer[0] = VertexArrayElement(screenX + screenWidth, screenY + screenHeight, bgTexX + bgTexWidth, bgTexY + bgTexHeight);
	buffer[1] = VertexArrayElement(screenX + screenWidth, screenY, bgTexX + bgTexWidth, bgTexY);
	buffer[2] = VertexArrayElement(screenX, screenY + screenHeight, bgTexX, bgTexY + bgTexHeight);
	
	buffer[3] = VertexArrayElement(screenX, screenY, bgTexX, bgTexY);
	buffer[4] = VertexArrayElement(screenX, screenY + screenHeight, bgTexX, bgTexY + bgTexHeight);
	buffer[5] = VertexArrayElement(screenX + screenWidth, screenY, bgTexX + bgTexWidth, bgTexY);	
}

void UIKnob::draw()
{	
	if (!isVisible) return;
	
	glVertexPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].posX));
	glTexCoordPointer(2, GL_FLOAT, sizeof(VertexArrayElement), &(buffer[0].texS));
	
	glDrawArrays(GL_TRIANGLES, 0, 12);
}

bool UIKnob::mouseDown(unsigned touchID, float x, float y)
{	
	if (currentTouch != UINT_MAX) return false;
	
	float centerX = screenX + 0.5f * screenWidth;
	float centerY = screenY + 0.5f * screenHeight;
	
	float diffX = x - centerX;
	float diffY = y - centerY;

	if (diffX*diffX + diffY*diffY > 0.25f*screenWidth*screenWidth) return false;
	
	currentTouch = touchID;
	
	return true;
}

bool UIKnob::mouseDragged(unsigned touchID, float x, float y)
{
	if (touchID != currentTouch) return false;
	
	float centerX = screenX + 0.5f * screenWidth;
	float centerY = screenY + 0.5f * screenHeight;
	
	float diffX = x - centerX;
	float diffY = y - centerY;
	
	float newAngle = -atan2f(diffX, diffY);
	setAngle(newAngle * 180.0f / float(M_PI));
	
	return true;
}

bool UIKnob::mouseUp(unsigned touchID, float x, float y)
{
	if (touchID != currentTouch) return false;
	
	currentTouch = UINT_MAX;
	
	return true;
}

void UIKnob::setAngle(float newAngle)
{
	setAngle(newAngle, true);
}

void UIKnob::setAngle(float newAngle, bool updateTarget)
{	
	angle = fmodf(newAngle, 360.0f);
	
	matrix rotation = matrix::rotation(float4(0, 0, 1, 0), angle * float(M_PI) / 180.0f);
	
	float4 corners[4] = {
		float4(-knobRadius, -knobRadius, 0, 1),
		float4(+knobRadius, -knobRadius, 0, 1),
		float4(+knobRadius, +knobRadius, 0, 1),
		float4(-knobRadius, +knobRadius, 0, 1)
	};
	
	float4 translation = float4(knobX, knobY, 0.0f, 0.0f);
	for (unsigned i = 0; i < 4; i++)
		corners[i] = (rotation * corners[i]) + translation;
	
	buffer[6] = VertexArrayElement(corners[2].x, corners[2].y, texX + texWidth, texY + texHeight);
	buffer[7] = VertexArrayElement(corners[1].x, corners[1].y, texX + texWidth, texY);
	buffer[8] = VertexArrayElement(corners[3].x, corners[3].y, texX, texY + texHeight);
	
	buffer[9] = VertexArrayElement(corners[0].x, corners[0].y, texX, texY);
	buffer[10] = VertexArrayElement(corners[3].x, corners[3].y, texX, texY + texHeight);
	buffer[11] = VertexArrayElement(corners[1].x, corners[1].y, texX + texWidth, texY);	
	
	if (updateTarget) target->knobAngleChanged(this, angle);
}
