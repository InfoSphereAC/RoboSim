/*
 *  RobotDrawer.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

class Drawer;
class Model;
class Robot;

class RobotDrawer
{
	Robot *robot;
	
	Model *body;
	Model *track;
	Model *wheel;
	
	Model *sensorArm;
	
	Model *lightSensor;
	Model *soundSensor;
	Model *touchSensor;
	Model *ultrasoundSensor;
	
	Model *flag;
	
	bool modelsLoaded;
	Drawer *drawer;
	
	float trackPositionLeft;
	float trackPositionRight;
	
	float wheelAngleLeft;
	float wheelAngleRight;
	
	float liftOffset;
	
public:
	RobotDrawer(Robot *robot, Drawer *drawer);
		
	void draw();
	
	Robot *getRobot() { return robot; }
	
	void update(float delta);
};
