/*
 *  SensorConfigurationScreen.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 22.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "SensorConfigurationScreen.h"

#include "NetworkInterface.h"
#include "UIIcon.h"
#include "UIKnob.h"
#include "UIRadioGroup.h"
#include "Robot.h"

namespace
{
	struct Rect
	{
		float x;
		float y;
		float width;
		float height;
	};
	
	const Rect numberIconTex[4] = {
		{80.0f, 202.0f, 37.0f, 48.0f},
		{41.0f, 202.0f, 37.0f, 48.0f}, 
		{ 2.0f, 202.0f, 37.0f, 48.0f},
		{ 2.0f, 252.0f, 37.0f, 48.0f},
	};
	const Rect numberIconScreen = { 0.0f, 0.0f, 37.0f, 48.0f };
	const Rect turntableIconScreen = { 297.0f, 0.0f, 51.0f, 48.0f };
	const Rect turntableIconTex = { 299.0f, 252.0f, 51.0f, 48.0f };
	
	const Rect buttonNoneScreen = {37.0f, 0, 38.0f, 48.0f};
	const Rect buttonNoneNormal = {39.0f, 252.0f, 38.0f, 48.0f};
	const Rect buttonNoneActive = {39.0f, 152.0f, 38.0f, 48.0f};
	const Rect buttonNoneSelected = {39.0f, 302.0f, 38.0f, 48.0f};
	
	const Rect buttonTouchScreen = {75.0f, 0, 43.0f, 48.0f};
	const Rect buttonTouchNormal = {77.0f, 252.0f, 43.0f, 48.0f};
	const Rect buttonTouchActive = {77.0f, 152.0f, 43.0f, 48.0f};
	const Rect buttonTouchSelected = {77.0f, 302.0f, 43.0f, 48.0f};
	
	const Rect buttonLightScreen = {118, 0, 50.0f, 48.0f};
	const Rect buttonLightNormal = {120.0f, 252.0f, 50.0f, 48.0f};
	const Rect buttonLightActive = {120.0f, 152.0f, 50.0f, 48.0f};
	const Rect buttonLightSelected = {120.0f, 302.0f, 50.0f, 48.0f};
	
	const Rect buttonSoundScreen = {168.0f, 0, 45.0f, 48.0f};
	const Rect buttonSoundNormal = {170.0f, 252.0f, 45.0f, 48.0f};
	const Rect buttonSoundActive = {170.0f, 152.0f, 45.0f, 48.0f};
	const Rect buttonSoundSelected = {170.0f, 302.0f, 45.0f, 48.0f};
	
	const Rect buttonUltrasoundScreen = {213.0f, 0, 50.0f, 48.0f};
	const Rect buttonUltrasoundNormal = {215.0f, 252.0f, 50.0f, 48.0f};
	const Rect buttonUltrasoundActive = {215.0f, 152.0f, 50.0f, 48.0f};
	const Rect buttonUltrasoundSelected = {215.0f, 302.0f, 50.0f, 48.0f};
	
	const Rect buttonDownScreen = {263.0f, 0, 34.0f, 28.0f};
	const Rect buttonDownNormal = {159.0, 202.0f, 34.0f, 28.0f};
	const Rect buttonDownNormalActive = {195.0f, 202.0f, 34.0f, 28.0f};
	const Rect buttonDownSelected = {231.0f, 202.0f, 34.0f, 28.0f};
	const Rect buttonDownSelectedActive = {267.0f, 202.0f, 34.0f, 28.0f};
	
	const Rect buttonAheadScreen = {263.0f, 28.0f, 34.0f, 20.0f};
	const Rect buttonAheadNormal = {159.0, 230.0f, 34.0f, 20.0f};
	const Rect buttonAheadNormalActive = {195.0f, 230.0f, 34.0f, 20.0f};
	const Rect buttonAheadSelected = {231.0f, 230.0f, 34.0f, 20.0f};
	const Rect buttonAheadSelectedActive = {267.0f, 230.0f, 34.0f, 20.0f};
	
	const Rect knobTex = {304.0f, 306.0f, 42.0f, 42.0f};
	const float knobCenterX = 323.0f;
	const float knobCenterY = 24.0f;
	const float knobRadius = 21.0f;
	
	const Rect dismissButtonScreen = {0, 190.0f, 20, 20};
	const Rect dismissButtonNormal = {128.0f, 225.0f, 20.0f, 20.0f};
	const Rect dismissButtonActive = {128.0f, 204.0f, 20.0f, 20.0f};
	
	const float panelX = 10.0f;
	const float panelY[] = {150, 100, 50, 0};
	
	// Output panel
	const float OCPPanelOffset = 3.0f;
	
	const float OCPButtonWidth = 10.0f;
	const float OCPButtonBottomC = 152.0f;
	const float OCPButtonAHeight = 49.0f;
	const float OCPButtonBHeight = 50.0f;
	const float OCPButtonCHeight = 48.0f;
	const float OCPPanelWidth = 56.0f;
	const float OCPPanelHeight = 198.0f;
	
	const float OCPGrayLeftStartX = 354.0f;
	const float OCPGrayRightStartX = 400.0f;
	const float OCPDarkGrayLeftStartX = 413.0f;
	const float OCPDarkGrayRightStartX = OCPDarkGrayLeftStartX + OCPButtonWidth;
	const float OCPOrangeLeftStartX = 436.0f;
	const float OCPOrangeRightStartX = OCPOrangeLeftStartX + OCPButtonWidth;
	const float OCPDarkOrangeLeftStartX = 459.0f;
	const float OCPDarkOrangeRightStartX = OCPDarkOrangeLeftStartX + OCPButtonWidth;
	
	const float OCPButtonBottomB = OCPButtonBottomC + OCPButtonCHeight;
	const float OCPButtonBottomA = OCPButtonBottomB + OCPButtonBHeight;
	
	const float OCPButtonBarHeight = OCPButtonAHeight + OCPButtonBHeight + OCPButtonCHeight;
	const Rect OCPTopIcon = { 353.0f, 301.0f, 56.0f, 50.0f };
	const Rect OCPBottomIcon = { 363.0f, 153.0f, 36.0f, 148.0f };
	
	// Overall constants
	
	const float panelWidth = 348.0f;
	const float panelHeight = 48.0f;
	const float configurationScreenWidth = panelWidth + 10.0f + OCPPanelOffset + OCPPanelWidth;
	const float configurationScreenHeight = 210.0f;
}


SensorConfigurationPanel::SensorConfigurationPanel(float x, float y, SensorConfigurationScreen *sensorScreen, NetworkInterface *anInterface, unsigned aSensor)
: UIContainer(x, y, panelWidth, panelHeight), networkInterface(anInterface), sensor(aSensor)
{
	// Create sensor type radio group
	UIRadioGroup *sensorTypeRadioGroup = new UIRadioGroup(0.0f, 0.0f, panelWidth, panelHeight, this);
	// No sensor
	sensorTypeRadioGroup->addButton(buttonNoneScreen.x, buttonNoneScreen.y, buttonNoneScreen.width, buttonNoneScreen.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(0, UIButton::Normal, buttonNoneNormal.x, buttonNoneNormal.y, buttonNoneNormal.width, buttonNoneNormal.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(0, UIButton::NormalActive, buttonNoneActive.x, buttonNoneActive.y, buttonNoneActive.width, buttonNoneActive.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(0, UIButton::Secondary, buttonNoneSelected.x, buttonNoneSelected.y, buttonNoneSelected.width, buttonNoneSelected.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(0, UIButton::SecondaryActive, buttonNoneActive.x, buttonNoneActive.y, buttonNoneActive.width, buttonNoneActive.height);
	// Light
	sensorTypeRadioGroup->addButton(buttonLightScreen.x, buttonLightScreen.y, buttonLightScreen.width, buttonLightScreen.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(1, UIButton::Normal, buttonLightNormal.x, buttonLightNormal.y, buttonLightNormal.width, buttonLightNormal.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(1, UIButton::NormalActive, buttonLightActive.x, buttonLightActive.y, buttonLightActive.width, buttonLightActive.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(1, UIButton::Secondary, buttonLightSelected.x, buttonLightSelected.y, buttonLightSelected.width, buttonLightSelected.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(1, UIButton::SecondaryActive, buttonLightActive.x, buttonLightActive.y, buttonLightActive.width, buttonLightActive.height);
	// Sound
	sensorTypeRadioGroup->addButton(buttonSoundScreen.x, buttonSoundScreen.y, buttonSoundScreen.width, buttonSoundScreen.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(2, UIButton::Normal, buttonSoundNormal.x, buttonSoundNormal.y, buttonSoundNormal.width, buttonSoundNormal.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(2, UIButton::NormalActive, buttonSoundActive.x, buttonSoundActive.y, buttonSoundActive.width, buttonSoundActive.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(2, UIButton::Secondary, buttonSoundSelected.x, buttonSoundSelected.y, buttonSoundSelected.width, buttonSoundSelected.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(2, UIButton::SecondaryActive, buttonSoundActive.x, buttonSoundActive.y, buttonSoundActive.width, buttonSoundActive.height);
	// Touch
	sensorTypeRadioGroup->addButton(buttonTouchScreen.x, buttonTouchScreen.y, buttonTouchScreen.width, buttonTouchScreen.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(3, UIButton::Normal, buttonTouchNormal.x, buttonTouchNormal.y, buttonTouchNormal.width, buttonTouchNormal.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(3, UIButton::NormalActive, buttonTouchActive.x, buttonTouchActive.y, buttonTouchActive.width, buttonTouchActive.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(3, UIButton::Secondary, buttonTouchSelected.x, buttonTouchSelected.y, buttonTouchSelected.width, buttonTouchSelected.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(3, UIButton::SecondaryActive, buttonTouchActive.x, buttonTouchActive.y, buttonTouchActive.width, buttonTouchActive.height);
	// Ultrasound
	sensorTypeRadioGroup->addButton(buttonUltrasoundScreen.x, buttonUltrasoundScreen.y, buttonUltrasoundScreen.width, buttonUltrasoundScreen.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(4, UIButton::Normal, buttonUltrasoundNormal.x, buttonUltrasoundNormal.y, buttonUltrasoundNormal.width, buttonUltrasoundNormal.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(4, UIButton::NormalActive, buttonUltrasoundActive.x, buttonUltrasoundActive.y, buttonUltrasoundActive.width, buttonUltrasoundActive.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(4, UIButton::Secondary, buttonUltrasoundSelected.x, buttonUltrasoundSelected.y, buttonUltrasoundSelected.width, buttonUltrasoundSelected.height);
	sensorTypeRadioGroup->setTextureRectForButtonState(4, UIButton::SecondaryActive, buttonUltrasoundActive.x, buttonUltrasoundActive.y, buttonUltrasoundActive.width, buttonUltrasoundActive.height);
	subviews.push_back(sensorTypeRadioGroup);
	
	// Create sensor heading radio group
	UIRadioGroup *sensorOrientationGroup = new UIRadioGroup(0.0f, 0.0f, panelWidth, panelHeight, this);
	// Pointed ahead
	sensorOrientationGroup->addButton(buttonAheadScreen.x, buttonAheadScreen.y, buttonAheadScreen.width, buttonAheadScreen.height);
	sensorOrientationGroup->setTextureRectForButtonState(0, UIButton::Normal, buttonAheadNormal.x, buttonAheadNormal.y, buttonAheadNormal.width, buttonAheadNormal.height);
	sensorOrientationGroup->setTextureRectForButtonState(0, UIButton::NormalActive, buttonAheadNormalActive.x, buttonAheadNormalActive.y, buttonAheadNormalActive.width, buttonAheadNormalActive.height);
	sensorOrientationGroup->setTextureRectForButtonState(0, UIButton::Secondary, buttonAheadSelected.x, buttonAheadSelected.y, buttonAheadSelected.width, buttonAheadSelected.height);
	sensorOrientationGroup->setTextureRectForButtonState(0, UIButton::SecondaryActive, buttonAheadSelectedActive.x, buttonAheadSelectedActive.y, buttonAheadSelectedActive.width, buttonAheadSelectedActive.height);
	// Pointed down
	sensorOrientationGroup->addButton(buttonDownScreen.x, buttonDownScreen.y, buttonDownScreen.width, buttonDownScreen.height);
	sensorOrientationGroup->setTextureRectForButtonState(1, UIButton::Normal, buttonDownNormal.x, buttonDownNormal.y, buttonDownNormal.width, buttonDownNormal.height);
	sensorOrientationGroup->setTextureRectForButtonState(1, UIButton::NormalActive, buttonDownNormalActive.x, buttonDownNormalActive.y, buttonDownNormalActive.width, buttonDownNormalActive.height);
	sensorOrientationGroup->setTextureRectForButtonState(1, UIButton::Secondary, buttonDownSelected.x, buttonDownSelected.y, buttonDownSelected.width, buttonDownSelected.height);
	sensorOrientationGroup->setTextureRectForButtonState(1, UIButton::SecondaryActive, buttonDownSelectedActive.x, buttonDownSelectedActive.y, buttonDownSelectedActive.width, buttonDownSelectedActive.height);
	subviews.push_back(sensorOrientationGroup);	
	
	// Create number icon
	subviews.push_back(new UIIcon(numberIconScreen.x, numberIconScreen.y, numberIconScreen.width, numberIconScreen.height, numberIconTex[sensor].x, numberIconTex[sensor].y, numberIconTex[sensor].width, numberIconTex[sensor].height, sensorScreen));
	
	// Create knob
	UIKnob *directionKnob = new UIKnob(turntableIconScreen.x, turntableIconScreen.y, turntableIconScreen.width, turntableIconScreen.height, knobCenterX, knobCenterY, knobRadius, this);
	directionKnob->setBackgroundTexture(turntableIconTex.x, turntableIconTex.y, turntableIconTex.width, turntableIconTex.height);
	directionKnob->setKnobTexture(knobTex.x, knobTex.y, knobTex.width, knobTex.height);
	subviews.push_back(directionKnob);
	
	// Set initial states
	updateData();
}

void SensorConfigurationPanel::selectionChanged(UIRadioGroup *group, unsigned index)
{
	if (group == subviews[0]) // Sensor type
		networkInterface->setSensorType(sensor, Robot::SensorType(index));
	else if (group == subviews[1]) // Sensor orientation
		networkInterface->setIsSensorPointedDown(sensor, bool(index==1));
}

void SensorConfigurationPanel::knobAngleChanged(UIKnob *knob, float newAngle)
{
	networkInterface->setSensorAngle(sensor, newAngle);
}

void SensorConfigurationPanel::updateData()
{
	UIRadioGroup *sensorTypeRadioGroup = reinterpret_cast<UIRadioGroup *> (subviews[0]);
	sensorTypeRadioGroup->setSelectedIndex(unsigned(networkInterface->getSensorType(sensor)));
	UIRadioGroup *sensorOrientationGroup = reinterpret_cast<UIRadioGroup *> (subviews[1]);
	sensorOrientationGroup->setSelectedIndex(unsigned(networkInterface->isSensorPointedDown(sensor)));
	UIKnob *directionKnob = reinterpret_cast<UIKnob *> (subviews[3]);
	directionKnob->setAngle(networkInterface->getSensorAngle(sensor));	
}

OutputConfigurationPanel::OutputConfigurationPanel(float x, float y, SensorConfigurationScreen *sensorScreen, NetworkInterface *anInterface)
: UIContainer(x, y, OCPPanelWidth, OCPPanelHeight), networkInterface(anInterface)
{
	UIRadioGroup *leftRadioGroup = new UIRadioGroup(1.0f, 0.0f, OCPButtonWidth, OCPButtonBarHeight, this);
	leftRadioGroup->addButton(0.0f, OCPButtonBHeight + OCPButtonCHeight, OCPButtonWidth, OCPButtonAHeight);
	leftRadioGroup->setTextureRectForButtonState(0, UIButton::Normal,			OCPGrayLeftStartX,			OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	leftRadioGroup->setTextureRectForButtonState(0, UIButton::NormalActive,		OCPDarkGrayLeftStartX,		OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	leftRadioGroup->setTextureRectForButtonState(0, UIButton::Secondary,		OCPOrangeLeftStartX,		OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	leftRadioGroup->setTextureRectForButtonState(0, UIButton::SecondaryActive,	OCPDarkOrangeLeftStartX,	OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	leftRadioGroup->addButton(0.0f, OCPButtonCHeight, OCPButtonWidth, OCPButtonBHeight);
	leftRadioGroup->setTextureRectForButtonState(1, UIButton::Normal,			OCPGrayLeftStartX,			OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	leftRadioGroup->setTextureRectForButtonState(1, UIButton::NormalActive,		OCPDarkGrayLeftStartX,		OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	leftRadioGroup->setTextureRectForButtonState(1, UIButton::Secondary,		OCPOrangeLeftStartX,		OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	leftRadioGroup->setTextureRectForButtonState(1, UIButton::SecondaryActive,	OCPDarkOrangeLeftStartX,	OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);	
	leftRadioGroup->addButton(0.0f, 0.0f, OCPButtonWidth, OCPButtonCHeight);
	leftRadioGroup->setTextureRectForButtonState(2, UIButton::Normal,			OCPGrayLeftStartX,			OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	leftRadioGroup->setTextureRectForButtonState(2, UIButton::NormalActive,		OCPDarkGrayLeftStartX,		OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	leftRadioGroup->setTextureRectForButtonState(2, UIButton::Secondary,		OCPOrangeLeftStartX,		OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	leftRadioGroup->setTextureRectForButtonState(2, UIButton::SecondaryActive,	OCPDarkOrangeLeftStartX,	OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	subviews.push_back(leftRadioGroup);
	
	UIRadioGroup *rightRadioGroup = new UIRadioGroup(OCPPanelWidth - OCPButtonWidth, 0.0f, OCPButtonWidth, OCPButtonBarHeight, this);
	rightRadioGroup->addButton(0.0f, OCPButtonBHeight + OCPButtonCHeight, OCPButtonWidth, OCPButtonAHeight);
	rightRadioGroup->setTextureRectForButtonState(0, UIButton::Normal,			OCPGrayRightStartX,			OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	rightRadioGroup->setTextureRectForButtonState(0, UIButton::NormalActive,	OCPDarkGrayRightStartX,		OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	rightRadioGroup->setTextureRectForButtonState(0, UIButton::Secondary,		OCPOrangeRightStartX,		OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	rightRadioGroup->setTextureRectForButtonState(0, UIButton::SecondaryActive,	OCPDarkOrangeRightStartX,	OCPButtonBottomA, OCPButtonWidth, OCPButtonAHeight);
	rightRadioGroup->addButton(0.0f, OCPButtonCHeight, OCPButtonWidth, OCPButtonBHeight);
	rightRadioGroup->setTextureRectForButtonState(1, UIButton::Normal,			OCPGrayRightStartX,			OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	rightRadioGroup->setTextureRectForButtonState(1, UIButton::NormalActive,	OCPDarkGrayRightStartX,		OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	rightRadioGroup->setTextureRectForButtonState(1, UIButton::Secondary,		OCPOrangeRightStartX,		OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);
	rightRadioGroup->setTextureRectForButtonState(1, UIButton::SecondaryActive,	OCPDarkOrangeRightStartX,	OCPButtonBottomB, OCPButtonWidth, OCPButtonBHeight);	
	rightRadioGroup->addButton(0.0f, 0.0f, OCPButtonWidth, OCPButtonCHeight);
	rightRadioGroup->setTextureRectForButtonState(2, UIButton::Normal,			OCPGrayRightStartX,			OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	rightRadioGroup->setTextureRectForButtonState(2, UIButton::NormalActive,	OCPDarkGrayRightStartX,		OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	rightRadioGroup->setTextureRectForButtonState(2, UIButton::Secondary,		OCPOrangeRightStartX,		OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	rightRadioGroup->setTextureRectForButtonState(2, UIButton::SecondaryActive,	OCPDarkOrangeRightStartX,	OCPButtonBottomC, OCPButtonWidth, OCPButtonCHeight);
	subviews.push_back(rightRadioGroup);
	
	UIIcon *topPartIcon = new UIIcon(0.0f, OCPButtonBarHeight, OCPPanelWidth, OCPPanelHeight - OCPButtonBarHeight,
									 OCPTopIcon.x, OCPTopIcon.y, OCPTopIcon.width, OCPTopIcon.height, sensorScreen); 
	UIIcon *bottomPartIcon = new UIIcon(OCPButtonWidth, 0.0f, OCPPanelWidth - 2.f * OCPButtonWidth, OCPButtonBarHeight,
										OCPBottomIcon.x, OCPBottomIcon.y, OCPBottomIcon.width, OCPBottomIcon.height, sensorScreen);
	subviews.push_back(topPartIcon);
	subviews.push_back(bottomPartIcon);
	
	// Setup state
	updateData();
}

void OutputConfigurationPanel::selectionChanged(UIRadioGroup *group, unsigned index)
{
	if (group == subviews[0]) // Left track
		networkInterface->setMotorForSide(index, 0);
	else if (group == subviews[1]) // Right track
		networkInterface->setMotorForSide(index, 1);
}

void OutputConfigurationPanel::updateData()
{
	if (!networkInterface) return;
	const Robot *localRobot = networkInterface->getLocalRobot();
	if (!localRobot) return;
	
	UIRadioGroup *leftMotorSelectionGroup = reinterpret_cast<UIRadioGroup *> (subviews[0]);
	UIRadioGroup *rightMotorSelectionGroup = reinterpret_cast<UIRadioGroup *> (subviews[1]);
	
	leftMotorSelectionGroup->setSelectedIndex(localRobot->getPortForLeftMotor());
	rightMotorSelectionGroup->setSelectedIndex(localRobot->getPortForRightMotor());
}

SensorConfigurationScreen::SensorConfigurationScreen(NetworkInterface *anInterface)
: UIContainer(0.0f, 0.0f, configurationScreenWidth, configurationScreenHeight), networkInterface(anInterface)
{
	isVisible = false;
	// Create Sensor Configuration panels
	for (unsigned i = 0; i < 4; ++i)
		subviews.push_back(new SensorConfigurationPanel(panelX, panelY[i], this, networkInterface, i));
	
	// Create Output Configuration panel
	subviews.push_back(new OutputConfigurationPanel(panelX + panelWidth + OCPPanelOffset, 0.0f, this, networkInterface));
	
	// Create dismiss button
	UIButton *dismissButton = new UIButton(this, dismissButtonScreen.x, dismissButtonScreen.y, dismissButtonScreen.width, dismissButtonScreen.height);
	dismissButton->setTextureRectForState(UIButton::Normal, dismissButtonNormal.x, dismissButtonNormal.y, dismissButtonNormal.width, dismissButtonNormal.height);
	dismissButton->setTextureRectForState(UIButton::NormalActive, dismissButtonActive.x, dismissButtonActive.y, dismissButtonActive.width, dismissButtonActive.height);
	subviews.push_back(dismissButton);
}

void SensorConfigurationScreen::setIsVisible(bool isit)
{
	isVisible = isit;
	if (isVisible)
	{
		for (unsigned i = 0; i < 4; i++)
		{
			reinterpret_cast<SensorConfigurationPanel *>(subviews[i])->updateData();
		}
		reinterpret_cast<OutputConfigurationPanel *>(subviews[4])->updateData();
	}
}

void SensorConfigurationScreen::iconDragged(UIIcon *icon, float deltaX, float deltaY)
{
	setOrigin(screenX + deltaX, screenY + deltaY);
}

void SensorConfigurationScreen::action(UIButton *button)
{
	// Only the dismiss button would ever call this
	isVisible = false;
}

float SensorConfigurationScreen::getConfigurationScreenWidth()
{
	return configurationScreenWidth - 10.0f;
}

