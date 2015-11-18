/*
 *  Motor.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 19.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Motor.h"

#include <cmath>
#include <algorithm>

namespace
{
	const float powerToSpeedFactor = 10.0f;
}

Motor::Motor()
: power(0.0f),turnLimit(0.0f), turnedSinceTurnLimitReset(0.0f), blockCounter(0.0f), rotationCounter(0.0f), syncState(NotSynced)
{
}

float Motor::turn(float delta) throw()
{	
	float turned = getSpeed() * delta;
	
	turnedSinceTurnLimitReset += turned;
	blockCounter += turned;
	rotationCounter += turned;
	
	return turned;
}

void Motor::resetAllCounters() throw()
{
	resetBlockCounter();
	resetRotationCounter();
	setTurnTarget(0.0f);
}

void Motor::resetBlockCounter() throw()
{
	blockCounter = 0.0f;
}

void Motor::resetRotationCounter() throw()
{
	rotationCounter = 0.0f;
}

void Motor::setBlockCounterValue(float val) throw()
{
	blockCounter = val;
}

void Motor::setTargetCounterValue(float val) throw()
{
	turnedSinceTurnLimitReset = val;
}

void Motor::setRotationCounterValue(float val) throw()
{
	rotationCounter = val;
}

float Motor::getBlockCounterValue() const throw()
{
	return blockCounter;
}

float Motor::getRotationCounterValue() const throw()
{
	return rotationCounter;
}

float Motor::getTargetCounterValue() const throw()
{
	return turnedSinceTurnLimitReset;
}

void Motor::setTurnTarget(float target) throw()
{
	turnLimit = fabsf(target);
	turnedSinceTurnLimitReset = 0.0f;
}

float Motor::getTurnTarget() const throw()
{
	return turnLimit;
}

void Motor::setTurnFactor(float factor) throw()
{
	robotTurnFactor = factor;
}
float Motor::getTurnFactor() const throw()
{
	return robotTurnFactor;
}

void Motor::setPower(float value) throw()
{
	power = value;
}

float Motor::getPower() const throw()
{
	return power;
}

float Motor::getSpeed() const throw()
{	
	float effectivePower = power;
	
	if (turnLimit > 0.0f && fabsf(turnedSinceTurnLimitReset) >= turnLimit)
		effectivePower = 0.0f;
	else if (syncState == IsLeft)
		effectivePower = power * std::min(std::max(-2.0f*robotTurnFactor + 1.f, -1.f), 1.f);
	else if (syncState == IsRight)
		effectivePower = power * std::min(std::max(2.0f*robotTurnFactor + 1.f, -1.f), 1.f);
	
	return effectivePower * powerToSpeedFactor;
}

bool Motor::isRunning() const throw()
{
	if (turnLimit > 0.0f && fabsf(turnedSinceTurnLimitReset) >= turnLimit)
		return false;
	if (power == 0.0f)
		return false;
	
	return true;
}

// Connections
void Motor::setIsLeftMotor() throw()
{
	syncState = IsLeft;
}
void Motor::setIsRightMotor() throw()
{
	syncState = IsRight;
}

void Motor::setIsNotSynchronized() throw()
{
	syncState = NotSynced;
}
