#ifndef ROBOT_NETWORK_INTERFACE_H
#define ROBOT_NETWORK_INTERFACE_H 1
/*
 *  NetworkInterface.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 06.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

#include "Robot.h" // For Robot::SensorType

class Drawer;
class SoundController;

/*!
 * @class NetworkInterface
 * @abstract Implements a basic set of functions for a robot to call if it wants to interact with a local or remote server.
 * @discussion All communication from a simulated to the server goes through this. It is an abstract class that is implemented by both Server (which directly executes the commands) and Client (which passes the commands to the server). In terms of architecture, it lies between the Interpreter/System and the robot. Notice that it is only one way: Client and Server call the Robot directly if they have anything to say to it.
*/

class NetworkInterface
{
private:
	Drawer *drawer;
	SoundController *soundController;
	
	float preservedMotorSpeeds[3];
		
protected:	
	/*!
	 * @abstract Called by a subclass whenever a new (visible) robot is added.
	 * @discussion This handles all the communication with the drawer and sound
	 * controller, to that the Server and Client don’t have to.
	 */
	void registerNewRobot(Robot *aRobot);
	
	/*!
	 * @abstract Called by a subclass whenever a (visible) robot is removed.
	 * @discussion This handles all the communication with the drawer and sound
	 * controller, to that the Server and Client don’t have to.
	 */
	void removeRobot(Robot *aRobot);
	
	/*!
	 * @abstract Allows enumerating of all robots.
	 * @discussion The subclass has to implement this so that all robots can
	 * be found, for example when adding a new drawer.
	 * @result The next robot, or NULL if all robots are found. If that is the
	 * case, starts with the first one again.
	 */
	virtual Robot *getNextRobot() throw() = 0;
	
	/*!
	 * @abstract Constructor.
	 * @discussion Just sets some stuff to zero. Nothing fancy here.
	 */
	NetworkInterface();
    
    /*!
     * @abstract Destructor
     * @discussion Not really necessary, just for cleanness.
     */
    virtual ~NetworkInterface() {}
	
	/*!
	 * @abstract Private read-write robot
	 */
	virtual Robot *getLocalModifiableRobot() throw() = 0;

public:
	
	/*!
	 * @abstract Sets the drawer that draws the robots.
	 * @discussion This is mainly necessary to tell the drawer when a new
	 * robot is added or removed. It is legal to pass in NULL here.
	 * @param aDrawer The drawer.
	 */
	void setDrawer(Drawer *aDrawer);
	/*!
	 * @abstract Current drawer.
	 */
	Drawer *getDrawer() { return drawer; }
	/*!
	 * @abstract Sets the sound controller that is responsible for all sound.
	 * @discussion This is mainly necessary to tell the controller when a new
	 * robot is added or removed. It is legal to pass in NULL here.
	 * @param aDrawer The drawer.
	 */
	void setSoundController(SoundController *aDrawer);
	
	/*!
	 * @abstract Sets a robot in pause mode.
	 * @discussion In pause mode, the speed of the robots motors is set to 0,
	 * and restored once pause mode ends. This method does not need to be
	 * overriden, but uses overriden ones in its implementation.
	 */
	void setIsPaused(bool pause) throw();
	
	/*!
	 * @abstract Stuff for the client to implement.
	 * @discussion This is usually used to synchronise the network.
	 */
	virtual void update() = 0;
	
	/*!
	 * @abstract The robot controlled by this computer.
	 * @discussion May return NULL if no such robot exists yet.
	 */
	virtual const Robot *getLocalRobot() const throw() = 0;
	
	// Called by the local robot to communicate with the server
	virtual void playTone(unsigned frequency, unsigned duration, bool loops, float gain) = 0;
	virtual void playFile(const char *name, bool loops, float gain) = 0;
	
	// Set motor data
	virtual void commitMotorChanges(unsigned atPort) throw (std::out_of_range);
	virtual void commitTurnTargetChanges(unsigned atPort) throw (std::out_of_range);
	virtual void resetAllCounters(unsigned atPort) throw (std::out_of_range);
	virtual void resetBlockCounter(unsigned atPort) throw (std::out_of_range);
	virtual void resetRotationCounter(unsigned atPort) throw (std::out_of_range);
	virtual void setMotorTurnTarget(unsigned atPort, float target) throw (std::out_of_range);
	virtual void setMotorTurnRatio(unsigned atPort, float ratio) throw (std::out_of_range);
	virtual void setMotorIsSynchronized(unsigned atPort, bool isit) throw (std::out_of_range);
	virtual void setMotorForSide(unsigned port, unsigned side) throw (std::out_of_range);
	virtual void setMotorPower(unsigned atPort, float power) throw (std::out_of_range);
	
	// Move robot
	virtual void setIsLifted(bool isRaised) throw() = 0;
	virtual void moveLifted(const union float4 &diff) throw() = 0;
	virtual void setRobotTurnSpeed(float turnSpeed) throw() = 0;
	virtual void rotateLifted(float radian) throw() = 0;
	
	// Configure sensor
	virtual void setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range) = 0;
	virtual void setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range) = 0;
	virtual void setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument) = 0;
	
	// Called by the environment editor to communicate with the server
	virtual void updatedCellState(unsigned x, unsigned z) = 0;
	
	// Not (usually) transmitted via the network.
	virtual float getSensorAngle(unsigned sensor) const throw(std::out_of_range);
	virtual bool isSensorPointedDown(unsigned sensor) const throw(std::out_of_range);
	virtual Robot::SensorType getSensorType(unsigned sensor) const throw(std::out_of_range);
};

#endif /* ROBOT_NETWORK_INTERFACE_H */
