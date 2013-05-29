/*
 *  UIRadioGroup.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 24.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef UI_RADIO_GROUP_H
#define UI_RADIO_GROUP_H

#include "UIButton.h"

class UIRadioGroup;

class UIRadioGroupTarget
{
public:
	/*!
	 * @abstract Called when the selection of a radio group changes.
	 * @discussion It is allowed to delete the group from within this method.
	 * @param group The group whose selection changed.
	 * @param newSelection The newly selected index.
	 */
	virtual void selectionChanged(UIRadioGroup *group, unsigned newSelection) = 0;
};

class UIRadioGroup : public UIContainer, UIButtonTarget
{
	int selectedIndex;
	UIRadioGroupTarget *target;
	
public:
	UIRadioGroup(float x, float y, float width, float height, UIRadioGroupTarget *aTarget) : UIContainer(x, y, width, height), selectedIndex(-1), target(aTarget) {}
	
	virtual void action(UIButton *sender);
	
	int getSelectedIndex() { return selectedIndex; }
	void setSelectedIndex(int index);
	
	void addButton(float x, float y, float width, float height);
	void addCircularButton(float x, float y, float width, float height);
	void setTextureRectForButtonState(unsigned button, UIButton::State state, float x, float y, float width, float height);
};

#endif /* UI_RADIO_GROUP_H */
