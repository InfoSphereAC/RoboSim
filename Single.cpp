/*
 *  Single.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Single.h"

#include "Robot.h"
#include "Simulation.h"

Single::Single(Simulation *aSimulation)
: simulation(aSimulation)
{
	robot = new Robot(simulation);
	registerNewRobot(robot);
	simulation->addRobot(robot);
	
	iteratorIsThrough = false;
}

Single::~Single()
{
	simulation->removeRobot(robot);
	removeRobot(robot);
	
	delete robot;
	robot = NULL;
}

// Network interface
void Single::update()
{
	// Ignore
}

void Single::playTone(unsigned frequency, unsigned duration, bool loops, float gain)
{
	robot->playTone(frequency, duration, loops, gain);
}

void Single::playFile(const char *name, bool loops, float gain)
{
	robot->playFile(name, loops, gain);
}

void Single::setIsLifted(bool isRaised) throw()
{
	robot->setIsLifted(isRaised);
}

void Single::moveLifted(const union float4 &diff) throw()
{
	robot->moveDirectly(diff);
}

void Single::rotateLifted(float radians) throw()
{	
	robot->rotate(radians);
}

void Single::setRobotTurnSpeed(float speed) throw()
{
	robot->setLiftedTurnSpeed(speed);
}

void Single::updatedCellState(unsigned x, unsigned z)
{
	// Ignore
}

void Single::setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range)
{
	robot->setSensorAngle(sensor, angleInDegrees);
}

void Single::setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range)
{
	robot->setIsSensorPointedDown(sensor, isit);
}

void Single::setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument)
{
	robot->setSensorType(sensor, type);
}

Robot *Single::getNextRobot() throw()
{
	iteratorIsThrough = !iteratorIsThrough;
	
	if (!iteratorIsThrough) return NULL;
	else return robot;
}

Robot *Single::getLocalModifiableRobot() throw()
{
	return robot;
}

const Robot *Single::getLocalRobot() const throw()
{
	return robot;
}

