/*
 *  Single.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "NetworkInterface.h"

class Robot;
class Simulation;

class Single : public NetworkInterface
{
protected:
	Robot *robot;
	Simulation *simulation;
	bool iteratorIsThrough;
	
	virtual Robot *getNextRobot() throw();
	virtual Robot *getLocalModifiableRobot() throw();
public:
	Single(Simulation *aSimulation);
	~Single();
	
	virtual void update();
	virtual void playTone(unsigned frequency, unsigned duration, bool loops, float gain);
	virtual void playFile(const char *name, bool loops, float gain);
	virtual void setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range);
	virtual void setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range);
	virtual void setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument);
	virtual void updatedCellState(unsigned x, unsigned z);
	virtual const Robot *getLocalRobot() const throw();
	virtual void setIsLifted(bool isRaised) throw();
	virtual void moveLifted(const union float4 &diff) throw();
	virtual void rotateLifted(float radians) throw();
	virtual void setRobotTurnSpeed(float turnSpeed) throw();
};
