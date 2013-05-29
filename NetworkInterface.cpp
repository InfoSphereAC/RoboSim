/*
 *  NetworkInterface.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 06.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "NetworkInterface.h"

#include "Drawer.h"
#include "Robot.h"
#include "SoundController.h"

NetworkInterface::NetworkInterface()
: drawer(0), soundController(0)
{
}

void NetworkInterface::registerNewRobot(Robot *aRobot)
{
	if (drawer) drawer->addRobot(aRobot);
	if (soundController) soundController->addRobot(aRobot);
}

void NetworkInterface::removeRobot(Robot *aRobot)
{
	if (drawer) drawer->removeRobot(aRobot);
	if (soundController) soundController->removeRobot(aRobot);
}

void NetworkInterface::setDrawer(Drawer *aDrawer)
{
	if (drawer) throw std::logic_error("Can’t change drawer later.");
	
	drawer = aDrawer;
	
	if (drawer)
	{
		while (Robot *aRobot = getNextRobot())
			drawer->addRobot(aRobot);
	}
}

void NetworkInterface::setSoundController(SoundController *aController)
{
	if (soundController) throw std::logic_error("Can’t change soundController later.");
	
	soundController = aController;
	
	if (soundController)
	{
		while (Robot *aRobot = getNextRobot())
			soundController->addRobot(aRobot);
	}
}

void NetworkInterface::setIsPaused(bool pause) throw()
{
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return;
	
	if (pause)
	{
		preservedMotorSpeeds[0] = localRobot->getMotor(0)->getSpeed();
		preservedMotorSpeeds[1] = localRobot->getMotor(1)->getSpeed();
		preservedMotorSpeeds[2] = localRobot->getMotor(2)->getSpeed();
		setMotorPower(0, 0.0f);
		setMotorPower(1, 0.0f);
		setMotorPower(2, 0.0f);
		commitMotorChanges(0);
		commitMotorChanges(1);
		commitMotorChanges(2);
	}
	else
	{
		setMotorPower(0, preservedMotorSpeeds[0]);
		setMotorPower(1, preservedMotorSpeeds[1]);
		setMotorPower(2, preservedMotorSpeeds[2]);
		commitMotorChanges(0);
		commitMotorChanges(1);
		commitMotorChanges(2);
	}
}

// Set motor data
// Actually, let’s just ignore the commit things for now.
void NetworkInterface::commitMotorChanges(unsigned atPort) throw (std::out_of_range) { }
void NetworkInterface::commitTurnTargetChanges(unsigned atPort) throw (std::out_of_range) { }

void NetworkInterface::resetAllCounters(unsigned atPort) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->resetAllCounters();
}

void NetworkInterface::resetBlockCounter(unsigned atPort) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->resetBlockCounter();
}

void NetworkInterface::resetRotationCounter(unsigned atPort) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->resetRotationCounter();
}

void NetworkInterface::setMotorTurnTarget(unsigned atPort, float target) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->setTurnTarget(target);
}

void NetworkInterface::setMotorTurnRatio(unsigned atPort, float ratio) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->setTurnFactor(ratio);
}

void NetworkInterface::setMotorIsSynchronized(unsigned atPort, bool isit) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->setMotorIsSynchronized(atPort, isit);
}

void NetworkInterface::setMotorPower(unsigned atPort, float power) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot) localRobot->getMotor(atPort)->setPower(power);
}

void NetworkInterface::setMotorForSide(unsigned atPort, unsigned side) throw (std::out_of_range)
{
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot)
	{
		if (side == 0) localRobot->setPortForLeftMotor(atPort);
		else if (side == 1) localRobot->setPortForRightMotor(atPort);
		else throw std::out_of_range("setMotorForSide: side is not in {0, 1}");
	}
}

float NetworkInterface::getSensorAngle(unsigned sensor) const throw(std::out_of_range)
{
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return 0.0f;
	
	return localRobot->getSensorAngle(sensor);
}

bool NetworkInterface::isSensorPointedDown(unsigned sensor) const throw(std::out_of_range)
{
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return false;
	
	return localRobot->isSensorPointedDown(sensor);
}

Robot::SensorType NetworkInterface::getSensorType(unsigned sensor) const throw(std::out_of_range)
{
	const Robot *localRobot = getLocalRobot();
	if (!localRobot) return Robot::None;
	
	return localRobot->getSensorType(sensor);	
}

