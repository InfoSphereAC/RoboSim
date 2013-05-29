/*
 *  UIRadioGroup.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 24.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UIRadioGroup.h"

void UIRadioGroup::action(UIButton *sender)
{
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		if (*iter == sender)
		{
			setSelectedIndex(iter - subviews.begin());
			return;
		}
	}
}

void UIRadioGroup::setSelectedIndex(int index)
{
	for (std::vector<UIView *>::iterator iter = subviews.begin(); iter != subviews.end(); ++iter)
	{
		reinterpret_cast<UIButton *>(*iter)->setState(UIButton::Normal);
	}
	
	selectedIndex = index;
	
	if (selectedIndex >= 0 && selectedIndex < int(subviews.size()))
	{
		reinterpret_cast<UIButton *>(subviews[index])->setState(UIButton::Secondary);
		if (target)
			target->selectionChanged(this, index);
	}
}

void UIRadioGroup::addButton(float x, float y, float width, float height)
{
	UIButton *newButton = new UIButton(this, x, y, width, height);
	subviews.push_back(newButton);
	newButton->setState(UIButton::Normal);
	
}

void UIRadioGroup::addCircularButton(float x, float y, float width, float height)
{
	UICircularButton *newButton = new UICircularButton(this, x, y, width, height);
	subviews.push_back(newButton);
	newButton->setState(UIButton::Normal);
}

void UIRadioGroup::setTextureRectForButtonState(unsigned button, UIButton::State state, float x, float y, float width, float height)
{
	if (button >= subviews.size()) return;
	
	reinterpret_cast<UIButton *>(subviews[button])->setTextureRectForState(state, x, y, width, height);
}