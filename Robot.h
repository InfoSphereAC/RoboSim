/*
 *  Robot.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 17.04.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#pragma once

#include <stdexcept>

#include "Motor.h"
#include "Vec4.h"

class Simulation;
class RobotSpeaker;

class Robot
{
public:
	enum SensorType {
		None,
		Light,
		Sound,
		Touch,
		Ultrasound
	};
private:
	Simulation *simulation;
	
	matrix position;
	matrix lastPosition; // To avoid tunnelling. Used to find how much the robot moved in the last step
	
	float yaw;
	
	Motor motors[3];
	unsigned leftMotor;
	unsigned rightMotor;
	bool speedsSynchronized[3];
	
	float leftTrackSpeed;
	float rightTrackSpeed;
	
	float4 obb[4];
	float4 aabb[2];
	
	RobotSpeaker *speaker;
		
	struct Sensor
	{
		SensorType type;
		float angle;
		bool isPointedDown;
		
		matrix relativePosition;
		float value;
	} sensors[4];
	
	bool lifted;
	float liftedTurnSpeed;
	
	float flagColor[3];
	
	bool isPaused;
public:
	/*!
	 * @abstract Constructs a robot.
	 * @discussion A robot object contains all the physical information for the
	 * robot, but not the code. On a server, this is quite a bit of data, while
	 * on the client, it is essentially only the matrix and the speeds of the
	 * tracks. For simplicity’s sake, both use the same class, though. On a more
	 * conceptual level, a server robot can use physical simulation, while a
	 * client can’t, so to differentiate, pass a NULL simulation argument for
	 * a client.
	 * @param simulation The physical simulation this robot is part of, or NULL
	 * if the robot object is not moved except from the outside. The robot does
	 * not take ownership of the simulation, it has to be manually deleted after
	 * the robot.
	 */
	Robot(Simulation *simulation);
	
	/*! Destructor. */
	~Robot();
	
	// TODO: Not very nicely implemented.
	void setSpeaker(RobotSpeaker *aSpeaker);
	
	/*!
	 * @abstract Updates the physics
	 * @discussion Moves the robot according to its speed settings.
	 * @param timedelta The time that has passed since the last time updatePhsics
	 * was called.
	 */
	void updatePhysics(float timedelta) throw();
	
	/*!
	 * @abstract Hit detection
	 * @discussion Tells whether a ray intersects the area currently occupied by
	 * this robot.
	 */
	bool hitByRay(const ray4 &ray, float &length) const throw();
	/*!
	 * @abstract Hit detection with wider outline
	 * @discussion Like hitByRay, but with a wider outline, to make it more easy to hit when using a touch device.
	 */
	bool touchHitByRay(const ray4 &ray, float &length, float scaleFactor) const throw();
	
	/*!
	 * @abstract Sets the type of a sensor
	 * @param sensor The sensor to set. Has to be in the [0; 3] range.
	 * @param type The type of the sensor.
	 * @throws std::invalid_argument If type is not a valid type or sensor is out
	 * of range.
	 */
	void setSensorType(unsigned sensor, SensorType type) throw(std::invalid_argument);
	/*!
	 * @abstract Returns the type of a sensor
	 * @param sensor The sensor to set. Has to be in the [0; 3] range.
	 * @result The type of the sensor.
	 * @throws std::out_of_range If sensor is out of range.
	 */
	SensorType getSensorType(unsigned sensor) const throw(std::out_of_range);
	
	/*!
	 * @abstract Sets whether the Robot is in lifted state.
	 * @discussion In lifted state, the motors no longer move the robot. It can
	 * and should also be drawn differently.
	 */
	void setIsLifted(bool isit) throw();
	bool isLifted() const throw();
	
	/* Turn the robot when it is lifted. */
	void setLiftedTurnSpeed(float speed) throw() { liftedTurnSpeed = speed; }
	float getLiftedTurnSpeed() const throw() { return liftedTurnSpeed; }
	
	/* Turn the robot a specified amount of radians */
	void rotate(float radians) throw();
		
	void setFlagColor(const float *color) throw();
	const float *getFlagColor() const throw();
	
	float getSensorOffset(unsigned sensor) const throw(std::out_of_range);
	void setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range);
	float getSensorAngle(unsigned sensor) const throw(std::out_of_range);
	bool isSensorPointedDown(unsigned sensor) const throw(std::out_of_range);
	void setIsSensorPointedDown(unsigned sensor, bool isit) throw(std::out_of_range);
	
	// Override for network
	void setSensorValue(unsigned sensor, float value) throw(std::out_of_range);
	float getSensorValue(unsigned sensor) const throw(std::out_of_range);
	
	
	// Sound output
	void playTone(unsigned frequency, unsigned durationInMilliseconds, bool repeats, float gain);
	void playFile(const char *filename, bool repeats, float gain);
	
	/*!
	 * @abstract Get the specified motor.
	 * @discussion A robot can have only three motors.
	 */
	Motor *getMotor(unsigned index) throw(std::out_of_range);
	/*!
	 * @abstract Get the specified motor.
	 * @abstract Overloaded method to be constant.
	 */
	const Motor *getMotor(unsigned index) const throw(std::out_of_range);
	
	/*!
	 * @abstract Assing which motor powers the left track.
	 * @discussion Can be the same as the right one.
	 */
	void setPortForLeftMotor(unsigned port) throw(std::out_of_range);
	/*!
	 * @abstract Which motor powers the left track.
	 */
	unsigned getPortForLeftMotor() const throw();
	
	/*!
	 * @abstract Assing which motor powers the right track.
	 * @discussion Can be the same as the left one.
	 */	
	void setPortForRightMotor(unsigned port) throw(std::out_of_range);
	/*!
	 * @abstract Assing which motor powers the right track.
	 */	
	unsigned getPortForRightMotor() const throw();
	
	/*!
	 * @abstract Sets a motor to synchronized status.
	 * @discussion Two motors can be synchronized. They will then use the same
	 * speed, except for possibly a steering factor. If three motors are selected
	 * for synchronization, only motors 0 and 1 will act synchronized.
	 */
	void setMotorIsSynchronized(unsigned port, bool isit) throw(std::out_of_range);
	/*!
	 * @abstract Is a given motor synchronized?
	 */
	bool getMotorIsSynchronized(unsigned port) const throw(std::out_of_range); 
	
	/*!
	 * @abstract Set track speed.
	 * @discussion Usually set based on motor speed in update, but can be set
	 * from elsewhere e.g. network as well.
	 */
	void setLeftTrackSpeed(float speed) throw();
	/*!
	 * @abstract Set track speed.
	 * @discussion Usually set based on motor speed in update, but can be set
	 * from elsewhere e.g. network as well.
	 */	
	void setRightTrackSpeed(float speed) throw();
	
	/*!
	 * @abstract Convenience method to get speed of left track.
	 */
	float getLeftTrackSpeed() const throw();
	/*!
	 * @abstract Convenience method to get speed of right track.
	 */
	float getRightTrackSpeed() const throw();
	
	const matrix &getPosition() const throw() { return position; }
	const matrix &getLastPosition() const throw() { return lastPosition; }
	
	const float4 *getOrientedBoundingBox() const throw() { return obb; }
	const float4 *getAxisAlignedBoundingBox() const throw() { return aabb; }
	
	void moveDirectly(const float4 &delta) throw();
	void setPosition(const matrix &position) throw();
};
