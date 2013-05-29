/*
 *  RobotDrawer.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "RobotDrawer.h"

#include <cmath>

#include "Drawer.h"
#include "Model.h"
#include "OpenGL.h"
#include "Robot.h"

/*!
 * Notes about this class’s implementation:
 *
 * A RobotDrawer will first of all greedily load the models for absolutely all
 * sensors, no matter what the robot actually needs. This is a little premature,
 * but it saves the trouble of having to find out what sensors the robot has at
 * any given moment and loading the sensors afterwards. In the future, that may
 * be a possible optimization, though. Notice that this method here does not
 * cause duplicate data loads, as the Drawer object makes sure that each model
 * file is loaded only once in total.
 *
 * The effect of moving tracks is implemented the easiest way possible: By
 * animating the texture matrix. This would probably be a prime candidate for a
 * vertex shader, but to support older systems, vertex shaders are right out.
 */

namespace
{
	const float trackZOffset = 0.825f;
	
	const float wheelXOffset = 0.65f;
	const float wheelYOffset = 0.2021f;
	
	const float wheelDiameter = 0.4f;
	
	const float sensorArmXOffset = 0.0f;
	const float sensorArmYOffset = 0.5f;
	
	const float maxLiftOffset = 1.0f;
	const float liftSpeed = 8.0f;
}

RobotDrawer::RobotDrawer(Robot *aRobot, Drawer *aDrawer) : robot(aRobot), drawer(aDrawer)
{
	modelsLoaded = false;
	
	liftOffset = 0;
	trackPositionLeft = 0;
	trackPositionRight = 0;
	wheelAngleLeft = 0;
	wheelAngleRight = 0;
}

void RobotDrawer::draw()
{
	if (!modelsLoaded)
	{
		body = drawer->modelWithFilename("robotbody.obj");
		track = drawer->modelWithFilename("robottrack.obj");
		wheel = drawer->modelWithFilename("robotwheel.obj");
		flag = drawer->modelWithFilename("robotflag.obj");
		
		sensorArm = drawer->modelWithFilename("sensorarm.obj");
		
		lightSensor = drawer->modelWithFilename("sensorlight.obj");
		soundSensor = drawer->modelWithFilename("sensorsound.obj");
		touchSensor = drawer->modelWithFilename("sensortouch.obj");
		ultrasoundSensor = drawer->modelWithFilename("sensorultrasound.obj");

		modelsLoaded = true;
	}
	
	glPushMatrix();
	glMultMatrixf(robot->getPosition().c_ptr());
	glTranslatef(0.0f, liftOffset, 0.0f);

	// Draw body
	body->setupAndDraw();

	// Draw flag
	glColor4f(robot->getFlagColor()[0], robot->getFlagColor()[1], robot->getFlagColor()[2], 1.0f);
	flag->setupAndDraw();
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

	// Draw sensors
	glPushMatrix();
	glTranslatef(sensorArmXOffset, sensorArmYOffset, 0.0f);

	for (unsigned i = 0; i < 4; i++)
	{
		if (robot->getSensorType(i) == Robot::None) continue;

		glPushMatrix();
		glRotatef(robot->getSensorAngle(i), 0.0f, 1.0f, 0.0f);
		sensorArm->setupAndDraw();
		glTranslatef(robot->getSensorOffset(i), 0.0f, 0.0f);
		if (robot->isSensorPointedDown(i)) glRotatef(-90.0f, 0.0f, 0.0f, 1.0f);
		switch (robot->getSensorType(i))
		{
			case Robot::Light:
				lightSensor->setupAndDraw();
				break;
			case Robot::Sound:
				soundSensor->setupAndDraw();
				break;
			case Robot::Touch:
				touchSensor->setupAndDraw();
				break;
			case Robot::Ultrasound:
				ultrasoundSensor->setupAndDraw();
				break;
			default:
				break;
		}
		glPopMatrix();
	}
	
	glPopMatrix();
	
	// Draw tracks:
	track->setup();
	glPushMatrix();
	// Left
	glTranslatef(0.0f, 0.0f, trackZOffset);

	glMatrixMode(GL_TEXTURE);
	glTranslatef(0.0f, trackPositionLeft, 0.0f);
	glMatrixMode(GL_MODELVIEW);
	
	track->draw();
	// Right
	glTranslatef(0.0f, 0.0f, -2.0f * trackZOffset);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glTranslatef(0.0f, trackPositionRight, 0.0f);
	glMatrixMode(GL_MODELVIEW);
	
	track->draw();
	glPopMatrix();

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	
	// Draw wheels:
	wheel->setup();

	// 1. LF
	glPushMatrix();
	glTranslatef(wheelXOffset, wheelYOffset, trackZOffset);
	glRotatef(wheelAngleLeft, 0.0f, 0.0f, 1.0f);
	wheel->draw();
	glPopMatrix();
	// 2. LR
	glPushMatrix();
	glTranslatef(wheelXOffset, wheelYOffset, -trackZOffset);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(wheelAngleLeft, 0.0f, 0.0f, 1.0f);
	wheel->draw();
	glPopMatrix();
	// 3. RF
	glPushMatrix();
	glTranslatef(-wheelXOffset, wheelYOffset, trackZOffset);
	glRotatef(wheelAngleRight, 0.0f, 0.0f, 1.0f);
	wheel->draw();
	glPopMatrix();
	// 4. RR
	glPushMatrix();
	glTranslatef(-wheelXOffset, wheelYOffset, -trackZOffset);
	glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
	glRotatef(wheelAngleRight, 0.0f, 0.0f, 1.0f);
	wheel->draw();
	glPopMatrix();
	
	glPopMatrix();
}

void RobotDrawer::update(float delta)
{
	trackPositionLeft = fmodf(trackPositionLeft - robot->getLeftTrackSpeed() * delta, 1.0f);
	trackPositionRight = fmodf(trackPositionRight - robot->getRightTrackSpeed() * delta, 1.0f);
	
	wheelAngleLeft = wheelAngleLeft - (robot->getLeftTrackSpeed() / wheelDiameter) * (float(M_PI) / 180.0f);
	wheelAngleRight = wheelAngleRight - (robot->getRightTrackSpeed() / wheelDiameter) * (float(M_PI) / 180.0f);
	
	if (getRobot()->isLifted())
		liftOffset = fminf(liftOffset + liftSpeed*delta, maxLiftOffset);
	else
		liftOffset = fmaxf(liftOffset - liftSpeed*delta, 0.0f);
}
