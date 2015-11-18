/*
 *  UserInterface.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 14.05.10.
 *	Edited by Thiemo Leonhardt in 2015
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "UserInterface.h"

#include "Controller.h"
#include "Drawer.h"
#include "EnvironmentEditor.h"
#include "NetworkInterface.h"
#include "OpenGL.h"
#include "SensorConfigurationScreen.h"
#include "Texture.h"
#include "UIIcon.h"

namespace
{
	const float buttonHeight = 64.0f;
	const float buttonWidth = 64.0f;
	const float nextButtonStride = 70.0f;
	const float buttonOffsetX = 10.0f;
	const float buttonOffsetY = 10.0f;
	
	const float texConversionFactor = 1.0f/512.0f;
	const float texButtonWidth = 64.0f;
	const float texButtonHeight = 64.0f;
	const float texButtonStartX = 0.0f;
	const float texButtonStartY = 512.0f - texButtonHeight;
	
	const float flagIconWidth = 64.0f;
	const float flagIconHeight = 63.0f;
	const float flagIconTexX = 288.0f;
	const float flagIconTexY = 129.0f;
	
	const float turnRobotIconWidth = 160.0f;
	const float turnRobotIconHeight = 31.0f;
	const float turnRobotIconTexX = 0.0f;
	const float turnRobotIconTexY = 352.0f;
	
	const float playModeButtonWidth = 128.0f;
	const float playModeButtonHeight = 128.0f;
	
	enum ViewItems
	{
		NetworkOrSingleSelector,
		PlayPauseButton,
		ReloadButton,
		FileOpenButton,
		ModeButtons,
		FlagIcon,
		TurnRobotIcon,
		ShowSensorSelectionButton,
		SensorConfiguration,
		MaxItems
	};
}

UserInterface::UserInterface(Controller *aController) :
	UIContainer(0.0f, 0.0f, 0.0f, 0.0f), controller(aController)
{
	texture = controller->getDrawer()->textureWithFilename("uigraphics.tga");
	controller->getDrawer()->setUserInterface(this);
	mouseIdentifier = UINT_MAX;
	
	subviews.resize(MaxItems);
	
	// Select single- or multiplayer
	if (controller->getNetworkInterface() == NULL)
	{
		subviews[NetworkOrSingleSelector] = new UIRadioGroup(0.0f, 0.0f, playModeButtonWidth * 2.0f, playModeButtonHeight, this);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->addButton(0.0f, 0.0f, playModeButtonWidth, playModeButtonHeight);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->setTextureRectForButtonState(0, UIButton::Normal, 0.0f, 0.0f, playModeButtonWidth, playModeButtonHeight);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->setTextureRectForButtonState(0, UIButton::NormalActive, playModeButtonWidth, 0.0f, playModeButtonWidth, playModeButtonHeight);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->addButton(playModeButtonWidth, 0.0f, playModeButtonWidth, playModeButtonHeight);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->setTextureRectForButtonState(1, UIButton::Normal, playModeButtonWidth*2.0f, 0.0f, playModeButtonWidth, playModeButtonHeight);
		((UIRadioGroup *) subviews[NetworkOrSingleSelector])->setTextureRectForButtonState(1, UIButton::NormalActive, playModeButtonWidth*3.0f, 0.0f, playModeButtonWidth, playModeButtonHeight);
	}
	else
		subviews[NetworkOrSingleSelector] = NULL;

	
	// Play/pause button
	subviews[PlayPauseButton] = new UICircularButton(this, 0.0f, 0.0f, buttonWidth, buttonHeight);
	((UIButton *) subviews[PlayPauseButton])->setTextureRectForState(UIButton::Normal, texButtonStartX + texButtonWidth, texButtonStartY, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[PlayPauseButton])->setTextureRectForState(UIButton::NormalActive, texButtonStartX + texButtonWidth, texButtonStartY - texButtonHeight, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[PlayPauseButton])->setTextureRectForState(UIButton::Secondary, texButtonStartX, texButtonStartY, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[PlayPauseButton])->setTextureRectForState(UIButton::SecondaryActive, texButtonStartX, texButtonStartY - texButtonHeight, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[PlayPauseButton])->setState(UIButton::Normal);
	subviews[PlayPauseButton]->setIsVisible(false);
	
	// Reload button
	subviews[ReloadButton] = new UICircularButton(this, 0.0f, 0.0f, buttonWidth, buttonHeight);
	((UIButton *) subviews[ReloadButton])->setTextureRectForState(UIButton::Normal, texButtonStartX + texButtonWidth*5.f, texButtonStartY, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[ReloadButton])->setTextureRectForState(UIButton::NormalActive, texButtonStartX + texButtonWidth*5.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[ReloadButton])->setState(UIButton::Normal);
	subviews[ReloadButton]->setIsVisible(false);
	
	// File open button
	subviews[FileOpenButton] = new UICircularButton(this, 0.0f, 0.0f, buttonWidth, buttonHeight);
	((UIButton *) subviews[FileOpenButton])->setTextureRectForState(UIButton::Normal, texButtonStartX + texButtonWidth*7.f, texButtonStartY, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[FileOpenButton])->setTextureRectForState(UIButton::NormalActive, texButtonStartX + texButtonWidth*7.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[FileOpenButton])->setState(UIButton::Normal);
	subviews[FileOpenButton]->setIsVisible(false);
	
	// Mode buttons
	subviews[ModeButtons] = new UIRadioGroup(-buttonOffsetX - buttonWidth - nextButtonStride*2.0f, -buttonOffsetY - buttonHeight, nextButtonStride*2.0f + buttonWidth, buttonHeight, this);
	// Height
	((UIRadioGroup *)subviews[ModeButtons])->addCircularButton(nextButtonStride*0.f, 0.0f, buttonWidth, buttonHeight);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(0, UIButton::Normal, texButtonStartX + texButtonWidth*2.f, texButtonStartY, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(0, UIButton::Secondary, texButtonStartX + texButtonWidth*2.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(0, UIButton::NormalActive, texButtonStartX + texButtonWidth*2.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	// Darken
	((UIRadioGroup *)subviews[ModeButtons])->addCircularButton(nextButtonStride*1.f, 0.0f, buttonWidth, buttonHeight);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(1, UIButton::Normal, texButtonStartX + texButtonWidth*4.f, texButtonStartY, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(1, UIButton::Secondary, texButtonStartX + texButtonWidth*4.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(1, UIButton::NormalActive, texButtonStartX + texButtonWidth*4.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	// Lighten
	((UIRadioGroup *)subviews[ModeButtons])->addCircularButton(nextButtonStride*2.f, 0.0f, buttonWidth, buttonHeight);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(2, UIButton::Normal, texButtonStartX + texButtonWidth*3.f, texButtonStartY, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(2, UIButton::Secondary, texButtonStartX + texButtonWidth*3.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	((UIRadioGroup *)subviews[ModeButtons])->setTextureRectForButtonState(2, UIButton::NormalActive, texButtonStartX + texButtonWidth*3.f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonWidth);
	subviews[ModeButtons]->setIsVisible(false);
	
	
	// Flag icon
	subviews[FlagIcon] = new UIIcon(-buttonOffsetX-flagIconWidth, buttonOffsetY, flagIconWidth, flagIconHeight, flagIconTexX, flagIconTexY, flagIconWidth, flagIconHeight);
	subviews[FlagIcon]->setIsVisible(false);
	
	// Robot turn icon
	subviews[TurnRobotIcon] = new UIIcon(-turnRobotIconWidth, -(buttonOffsetX + (buttonHeight + turnRobotIconHeight)*0.5f), turnRobotIconWidth, turnRobotIconHeight, turnRobotIconTexX, turnRobotIconTexY, turnRobotIconWidth, turnRobotIconHeight);
	subviews[TurnRobotIcon]->setIsVisible(false);
	
	// Show sensor configuration button
	subviews[ShowSensorSelectionButton] = new UICircularButton(this, buttonOffsetX, buttonOffsetY, buttonWidth, buttonHeight);
	((UIButton *) subviews[ShowSensorSelectionButton])->setTextureRectForState(UIButton::Normal, texButtonStartX + texButtonWidth * 6.0f, texButtonStartY, texButtonWidth, texButtonHeight);
	((UIButton *) subviews[ShowSensorSelectionButton])->setTextureRectForState(UIButton::NormalActive, texButtonStartX + texButtonWidth * 6.0f, texButtonStartY - texButtonHeight, texButtonWidth, texButtonHeight);
	subviews[ShowSensorSelectionButton]->setIsVisible(false);
	
	// Sensor configuration
	subviews[SensorConfiguration] = NULL;
	
	controller->getEnvironmentEditor()->setMode(EnvironmentEditor::None); // Default is None
	((UIRadioGroup *) subviews[ModeButtons])->setSelectedIndex(0);
}

UserInterface::~UserInterface()
{
}

void UserInterface::resize(float width, float height)
{
	screenWidth = width;
	screenHeight = height;
	
	// Set sizes
	if (subviews[NetworkOrSingleSelector])
	{
		subviews[NetworkOrSingleSelector]->setOrigin((width-playModeButtonWidth*2.f)/2.f, (height-playModeButtonHeight)/2.f);
	}
	subviews[PlayPauseButton]->setOrigin(buttonOffsetX, height-(buttonOffsetY+buttonHeight));
	subviews[ReloadButton]->setOrigin(buttonOffsetX+nextButtonStride, height-(buttonOffsetY+buttonHeight));
	subviews[FileOpenButton]->setOrigin(buttonOffsetX+2.f*nextButtonStride, height-(buttonOffsetY+buttonHeight));
	if (!controller->getEnvironmentEditor()->getChallenge()) subviews[ModeButtons]->setOrigin(width - (buttonOffsetX + 3.f*nextButtonStride), height - (buttonOffsetY + buttonHeight));
	subviews[FlagIcon]->setOrigin(width-(buttonOffsetX+nextButtonStride), buttonOffsetY);
	subviews[TurnRobotIcon]->setOrigin((width-turnRobotIconWidth)/2.f, height-(buttonOffsetY+(buttonHeight+turnRobotIconHeight)/2.f));
	
	if (subviews[SensorConfiguration])
	{
		subviews[SensorConfiguration]->setOrigin((screenWidth - SensorConfigurationScreen::getConfigurationScreenWidth()) / 2.0f, buttonOffsetY);
		subviews[ShowSensorSelectionButton]->setIsVisible(true);
		subviews[ShowSensorSelectionButton]->setOrigin(buttonOffsetX, buttonOffsetY);
	}
}

void UserInterface::draw(float width, float height)
{
	texture->set();
	glMatrixMode(GL_TEXTURE);
	glScalef(texConversionFactor, texConversionFactor, texConversionFactor);
	glMatrixMode(GL_MODELVIEW);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	if (width != screenWidth || height != screenHeight)
	{
		resize(width, height);
	}
	if (controller->getNetworkInterface())
	{
		if (!subviews[SensorConfiguration])
		{
			subviews[SensorConfiguration] = new SensorConfigurationScreen(controller->getNetworkInterface());

			subviews[PlayPauseButton]->setIsVisible(true);
			subviews[ReloadButton]->setIsVisible(true);
			subviews[FileOpenButton]->setIsVisible(true);
			if (!controller->getEnvironmentEditor()->getChallenge()) subviews[ModeButtons]->setIsVisible(true); // No edit in challenge mode
			subviews[FlagIcon]->setIsVisible(true);
			subviews[ShowSensorSelectionButton]->setIsVisible(true);
			
			resize(width, height);
		}
		
		const Robot *localRobot = controller->getLocalRobot();
		if (localRobot)
		{
#if !defined(IPHONE) && !defined(ANDROID_NDK)
			subviews[TurnRobotIcon]->setIsVisible(localRobot->isLifted());
#endif
			((UIIcon *) subviews[FlagIcon])->setColor(localRobot->getFlagColor());
		}
	}
	
	// Draw
	UIContainer::draw();
	
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
}

void UserInterface::action(UIButton *aButton)
{
	if (aButton == subviews[PlayPauseButton])
	{
		controller->togglePause();

		if (controller->getIsPaused()) aButton->setState(UIButton::Secondary);
		else aButton->setState(UIButton::Normal);
	}
	else if (aButton == subviews[ReloadButton])
	{
		controller->reload();
	}
	else if (aButton == subviews[FileOpenButton])
	{
		controller->chooseFile();
	}
	else if (aButton == subviews[ShowSensorSelectionButton])
	{
		if (subviews[SensorConfiguration])
			subviews[SensorConfiguration]->setIsVisible(!subviews[SensorConfiguration]->getIsVisible());
	}
}

void UserInterface::toggleShowSensorScreen()
{
	if (!subviews[SensorConfiguration]) return;
	
	subviews[SensorConfiguration]->setIsVisible(!subviews[SensorConfiguration]->getIsVisible());
}

void UserInterface::selectionChanged(UIRadioGroup *group, unsigned index)
{
	if (group == subviews[NetworkOrSingleSelector])
	{
		controller->modeSelected(index == 0);
		
		delete subviews[NetworkOrSingleSelector];
		subviews[NetworkOrSingleSelector] = 0;
	}
	else if (group == subviews[ModeButtons])
	{
		if (!controller->getEnvironmentEditor()->getChallenge()) controller->getEnvironmentEditor()->setMode(EnvironmentEditor::EditingMode(index + 1)); 
	}
}
