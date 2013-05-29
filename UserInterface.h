/*
 *  UserInterface.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 14.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>

#include "UIRadioGroup.h"

class Controller;
class Texture;
class UIIcon;

class UserInterface : public UIContainer, public UIButtonTarget, public UIRadioGroupTarget
{
protected:
	Controller *controller;
	
	// Ensure only one single tap is active for the whole user interface. This may need to be expanded later.
	unsigned mouseIdentifier;
	
	Texture *texture;
	
	void action(UIButton *aButton);
		
	void fillModeButtons();
	void resize(float width, float height);
	
public:
	UserInterface(Controller *aController);
	virtual ~UserInterface();
	
	void draw(float screenWidth, float screenHeight);
	
	void toggleShowSensorScreen();
	
	virtual void selectionChanged(UIRadioGroup *group, unsigned index);
};
