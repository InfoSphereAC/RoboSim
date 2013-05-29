/*
 *  SensorConfigurationScreen.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIButton.h"
#include "UIIcon.h"
#include "UIKnob.h"
#include "UIRadioGroup.h"

class NetworkInterface;
class SensorConfigurationScreen;

/*!
 * @abstract User Interface for configuring one sensor.
 */
class SensorConfigurationPanel : public UIContainer, public UIRadioGroupTarget, public UIKnobTarget
{
	NetworkInterface *networkInterface;
	unsigned sensor;
	
public:
	SensorConfigurationPanel(float x, float y, SensorConfigurationScreen *sensorScreen, NetworkInterface *anInterface, unsigned sensor);
	
	virtual void selectionChanged(UIRadioGroup *group, unsigned index);
	virtual void knobAngleChanged(UIKnob *knob, float newAngle);
	
	void updateData();
};

/*!
 * @abtract Manages which port maps to which physical motor.
 */
class OutputConfigurationPanel : public UIContainer, public UIRadioGroupTarget
{
	NetworkInterface *networkInterface;
	
public:
	OutputConfigurationPanel(float x, float y, SensorConfigurationScreen *sensorScreen, NetworkInterface *anInterface);
	
	virtual void selectionChanged(UIRadioGroup *group, unsigned index);
	
	void updateData();
};

/*!
 * @abstract User Interface for configuring all sensors
 */
class SensorConfigurationScreen : public UIContainer, public UIButtonTarget, public UIIconDragTarget
{
	NetworkInterface *networkInterface;
	
public:
	static float getConfigurationScreenWidth();
	
	void action(UIButton *button);
	
	SensorConfigurationScreen(NetworkInterface *anInterface);
		
	virtual void setIsVisible(bool isit);
	virtual void iconDragged(UIIcon *icon, float deltaX, float deltaY);
	
	NetworkInterface *getNetworkInterface() { return networkInterface; }
};
