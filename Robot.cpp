/*
 *  Robot.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Robot.h"

#include <cmath>

#include "Simulation.h"
#include "SoundController.h"
#include "RobotSpeaker.h"

namespace
{
	const float trackWidth = 0.825f*2.0f;
	
	const float4 standard2DBoundingBox[4] = {
		float4(1.0,    0.0f, -0.9f),
		float4(1.0,    0.0f,  0.9f),
		float4(-0.85f, 0.0f,  0.9f),
		float4(-0.85f, 0.0f, -0.9f)
	};
	
	const float4 touchSensor2DBoundingBox[4] = {
		float4(0.45f,  0.0f, -0.125f),
		float4(0.45f,  0.0f,  0.125f),
		float4(0.375f, 0.0f,  0.125f),
		float4(0.375f, 0.0f, -0.125f)
	};
	
	const float boundingBox3DHeight = 1.1f;
	
	const float ultrasoundRange = 1000.0f;
	const float ultrasoundDistanceScale = 5.f;
	const float noiseLevelScale = 10.0f;
	const float touchRange = 0.25f;
	const float lightRange = 100.0f;
	
	const float sensorYOffset = 0.2f;
	const float sensorXOffset = 1.15f;
	
	const float wheelDiameter = 0.4f;
}

Robot::Robot(Simulation *aSimulation)
: simulation(aSimulation)
{
	yaw = 1;
	
	position.w = float4(10, 0, 10);
	leftMotor = 0;
	rightMotor = 1;
	
	setSensorAngle(0, -30.0f);
	setSensorType(0, Touch);
	sensors[0].isPointedDown = false;
	
	setSensorAngle(1, 30.0f);
	setSensorType(1, Touch);
	sensors[1].isPointedDown = false;

	setSensorAngle(2, 5.0f);
	setSensorType(2, Light);
	sensors[2].isPointedDown = true;
	
	setSensorAngle(3, -5.0f);
	setSensorType(3, Ultrasound);
	sensors[3].isPointedDown = false;
	
	speaker = NULL;
	lifted = false;
	
	flagColor[0] = 1.0f;
	flagColor[1] = 0.0f;
	flagColor[2] = 0.0f;
	
	liftedTurnSpeed = 0.0f;
}

Robot::~Robot()
{
	setSpeaker(NULL);
}

void Robot::setSpeaker(RobotSpeaker *aSpeaker)
{
	if (speaker)
	{
		delete speaker;
		speaker = NULL;
	}
	
	speaker = aSpeaker;
}

void Robot::updatePhysics(float timedelta) throw()
{
	if (!simulation) // Purely client robot.
		return;
	
	lastPosition = position;
	
	// Update speed data
	setLeftTrackSpeed(motors[leftMotor].getSpeed() * wheelDiameter);
	setRightTrackSpeed(motors[rightMotor].getSpeed() * wheelDiameter);	
	
	if (!lifted)
	{
		// Move according to position
		float deltaSleft = motors[leftMotor].turn(timedelta) * (float(M_PI) / 360.0f) * wheelDiameter;
		float deltaSright = motors[rightMotor].turn(timedelta) * (float(M_PI) / 360.0f) * wheelDiameter;
		
		float deltaStotal = (deltaSleft + deltaSright) * 0.5f;
		
		float deltaYaw = std::atan((deltaSleft - deltaSright) / trackWidth);
		
		yaw = fmodf(yaw + deltaYaw, 2.0f * float(M_PI));
		
		matrix newPosition(matrix::rotation(float4(0, 1, 0, 0), yaw));
		newPosition.w = position.w + position.x * deltaStotal;
		setPosition(newPosition);
	}
	else
	{
		// Turn according to speed
		rotate(liftedTurnSpeed * timedelta);
	}

	// Update sensor readings
	for (unsigned i = 0; i < 4; i++)
	{
		sensors[i].value = 0.0f;
		if (sensors[i].type == None) continue;
		
		matrix sensorLocation = position * sensors[i].relativePosition;
		
		float4 sensorDirection = isSensorPointedDown(i) ? -sensorLocation.y : sensorLocation.x;
		
		if (sensors[i].type == Light)
		{
			// Return in a range from 0 to 1
			float4 rayDirection = sensorDirection * lightRange;
			float length;
			if (!simulation->firstHitOfRay(ray4(sensorLocation.w, sensorLocation.w + rayDirection), true, length))
				continue;
			
			float4 target = sensorLocation.w + sensorDirection * length;
			unsigned x = unsigned(target.x);
			unsigned z = unsigned(target.z);
			
			sensors[i].value = simulation->getCellShade(x, z);
		}
		else if (sensors[i].type == Touch)
		{
			// Return 1 if something is hit, 0 otherwise
			float4 orientedBoundingBox[4];
			for (unsigned j = 0; j < 4; j++)
				orientedBoundingBox[j] = sensorLocation * touchSensor2DBoundingBox[j];
			
			bool doesHit = simulation->objectCollidesWithOthers(orientedBoundingBox);
			sensors[i].value = float(doesHit);
		}
		else if (sensors[i].type == Sound)
		{
			sensors[i].value = speaker->getSoundController()->noiseLevelAtPoint(sensorLocation.w) * noiseLevelScale;
		}
		else if (sensors[i].type == Ultrasound)
		{
			// Return length in centimeters
			float4 rayDirection = sensorDirection * ultrasoundRange;
			float length;
			
			if (!simulation->firstHitOfRay(ray4(sensorLocation.w, sensorLocation.w + rayDirection), false, length))
				continue;
			
			sensors[i].value = (length * ultrasoundRange) * ultrasoundDistanceScale;
		}		
	}
	
	if (speaker) speaker->update();
}

bool Robot::touchHitByRay(const ray4 &ray, float &length, float scaleFactor) const throw()
{
	// Transform ray into robot space
	ray4 relativeRay = position.inverse() * ray;

	// Find max and min coordinates in robot space
	float4 max(standard2DBoundingBox[1].x, boundingBox3DHeight, standard2DBoundingBox[1].z);
	float4 min(standard2DBoundingBox[3].x, 0.0f, standard2DBoundingBox[3].z);
	
	max *= scaleFactor;
	min *= scaleFactor;

	max.w = 1.0f;
	min.w = 1.0f;
	
	float exit;
	return relativeRay.hitsAABB(min, max, length, exit);
}

bool Robot::hitByRay(const ray4 &ray, float &length) const throw()
{
	return touchHitByRay(ray, length, 1.0f);
}

void Robot::setIsLifted(bool isit) throw()
{
	lifted = isit;
}

bool Robot::isLifted() const throw()
{
	return lifted;
}

void Robot::setPosition(const matrix &newLocation) throw()
{
	position = newLocation;
	
	// Find bounding boxes
	for (unsigned i = 0; i < 4; i++)
		obb[i] = position * standard2DBoundingBox[i];
	
	aabb[0] = obb[0].min(obb[1].min(obb[2].min(obb[3])));
	aabb[1] = obb[0].max(obb[1].max(obb[2].max(obb[3])));
}

void Robot::moveDirectly(const float4 &delta) throw()
{
	position.w += delta;
}

void Robot::rotate(float radians) throw()
{
	yaw = fmodf(yaw + radians, 2.0f * float(M_PI));
	matrix newPosition(matrix::rotation(float4(0, 1, 0, 0), yaw));
	newPosition.w = position.w;
	setPosition(newPosition);
}

void Robot::setFlagColor(const float *color) throw()
{
	memcpy(flagColor, color, sizeof(float [3]));
}
const float *Robot::getFlagColor() const throw()
{
	return flagColor;
}

void Robot::playTone(unsigned frequency, unsigned durationInMilliseconds, bool repeats, float gain)
{
	if (speaker) speaker->playTone(frequency, durationInMilliseconds, repeats, gain);
}
void Robot::playFile(const char *filename, bool repeats, float gain)
{
	if (speaker) speaker->playFile(filename, repeats, gain);
}

void Robot::setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument)
{
	if (sensor > 3) throw std::invalid_argument("Sensor port out of range");
	if (type > Ultrasound) throw std::invalid_argument("Invalid sensor type");
	
	sensors[sensor].type = type;
}

Robot::SensorType Robot::getSensorType(unsigned sensor) const throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	return sensors[sensor].type;
}
float Robot::getSensorOffset(unsigned sensor) const throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	return sensorXOffset;
}

void Robot::setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");

	sensors[sensor].angle = angleInDegrees;
	
	matrix rotationMatrix = matrix::rotation(float4(0, 1, 0, 0), angleInDegrees * (float(M_PI) / 180.0f));
	matrix positionMatrix = matrix::position(float4(getSensorOffset(sensor), sensorYOffset, 0));
	
	sensors[sensor].relativePosition = rotationMatrix * positionMatrix;
}

float Robot::getSensorAngle(unsigned sensor) const throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	return sensors[sensor].angle;
}

void Robot::setIsSensorPointedDown(unsigned sensor, bool isit) throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	sensors[sensor].isPointedDown = isit;
}

bool Robot::isSensorPointedDown(unsigned sensor) const throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	return sensors[sensor].isPointedDown;
}

float Robot::getSensorValue(unsigned sensor) const throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	return sensors[sensor].value;
}

void Robot::setSensorValue(unsigned sensor, float value) throw(std::out_of_range)
{
	if (sensor > 3) throw std::out_of_range("Sensor port out of range");
	
	sensors[sensor].value = value;
}

Motor *Robot::getMotor(unsigned motor) throw(std::out_of_range)
{
	if (motor > 2) throw std::out_of_range("Motor port out of range.");
	return &(motors[motor]);
}
const Motor *Robot::getMotor(unsigned motor) const throw(std::out_of_range)
{
	if (motor > 2) throw std::out_of_range("Motor port out of range.");
	return &(motors[motor]);	
}

void Robot::setPortForLeftMotor(unsigned motor) throw(std::out_of_range)
{
	if (motor > 2) throw std::out_of_range("Motor port out of range.");
	leftMotor = motor;
}

void Robot::setPortForRightMotor(unsigned motor) throw(std::out_of_range)
{
	if (motor > 2) throw std::out_of_range("Motor port out of range.");
	rightMotor = motor;
}

void Robot::setMotorIsSynchronized(unsigned port, bool isit) throw(std::out_of_range)
{
	if (port > 2) throw std::out_of_range("Motor port out of range.");
	
	speedsSynchronized[port] = isit;
	
	motors[0].setIsNotSynchronized();
	motors[1].setIsNotSynchronized();
	motors[2].setIsNotSynchronized();
	
	// Update what motor is synchronized to what and which port is seen as which.
	// This is completely independent of the motor port assignment! It reflects
	// only what the firmware thinks is left and right, not what the actual
	// physical reality (or here simulation) thinks.
	if (speedsSynchronized[0] && speedsSynchronized[1])
	{
		motors[0].setIsLeftMotor();
		motors[1].setIsRightMotor();
	}
	else if (speedsSynchronized[1] && speedsSynchronized[2])
	{
		motors[1].setIsLeftMotor();
		motors[2].setIsRightMotor();
	}
	else if (speedsSynchronized[0] && speedsSynchronized[2])
	{
		motors[0].setIsLeftMotor();
		motors[2].setIsRightMotor();
	}
}

bool Robot::getMotorIsSynchronized(unsigned port) const throw(std::out_of_range)
{
	if (port > 2) throw std::out_of_range("Motor port out of range.");
	
	return speedsSynchronized[port];
}

unsigned Robot::getPortForLeftMotor() const throw()
{
	return leftMotor;
}

unsigned Robot::getPortForRightMotor() const throw()
{
	return rightMotor;
}

void Robot::setLeftTrackSpeed(float value) throw()
{
	leftTrackSpeed = value;
}

void Robot::setRightTrackSpeed(float value) throw()
{
	rightTrackSpeed = value;
}

float Robot::getLeftTrackSpeed() const throw()
{
	return leftTrackSpeed;
}

float Robot::getRightTrackSpeed() const throw()
{
	return rightTrackSpeed;
}
