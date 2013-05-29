/*
 *  Motor.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 19.11.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdexcept>

/*!
 * @abstract Implements a motor as seen from Mindstorms code.
 * @discussion This class takes all settings for a particular output and uses
 * them to calculate the correct rotation, as well as giving values for the
 * various rotation counters.
 *
 * There are two counters, the rotation counter and the block counter. Both are 
 * independent but do the same thing. The names are from Lego documentation,
 * which assigns different default purposes for them. The turn target counter
 * will disable the motor once it is exceeded.
 *
 * It is your choice whether the angles are in degrees or radians. The speed
 * is then specified in degrees/second or radians/second.
 *
 * Notice that not all properties are supported, because they could not be
 * represented in the physics model anyway.
 */
class Motor
{
	float power;
	float turnLimit;
	float turnedSinceTurnLimitReset;
	
	// Both identical, they are just used differently by the system.
	float blockCounter;
	float rotationCounter;
	
	enum {
		NotSynced,
		IsLeft,
		IsRight
	} syncState;
	float robotTurnFactor;
public:
	/*!
	 * @abstract Constructor
	 * @discussion Sets speed, limit and counters all to zero.
	 */
	Motor();
	
	/*!
	 * @abstract Updates the motor state and returns how far it turned.
	 * @discussion This updates the counters and turns the motor according
	 * to the speed specified. If the motor has exceeded its turn limit, it
	 * does not turn.
	 * @param time Time in seconds
	 * @result How far the motor turned, in degrees, if at all.
	 */
	float turn(float time) throw();
	
	/*!
	 * @abstract Sets relations between motors.
	 * @discussion When synchronization and turning are used, then two motors
	 * are paired, with one treated as the left and one as the right motor of
	 * said pair. This method sets up such relationships.
	 */
	void setIsLeftMotor() throw();
	void setIsRightMotor() throw();
	void setIsNotSynchronized() throw();
	
	/*!
	 * @abstract Sets the turn factor for synchronized motors.
	 * @discussion Has to be set on both motors.
	 */
	void setTurnFactor(float factor) throw();
	
	/*!
	 * @abstract Turn factor for synchronized motors
	 */
	float getTurnFactor() const throw();
	
	/*!
	 * @abstract Resets all counters.
	 * @discussion This resets both counters and clears the current rotation
	 * target.
	 */
	void resetAllCounters() throw();
	/*!
	 * @abstract Resets block-specific counter.
	 */
	void resetBlockCounter() throw();
	/*!
	 * @abstract Rests program-specific counter.
	 */
	void resetRotationCounter() throw();
	/*!
	 * @abstract Current value of block-specific counter.
	 */
	float getBlockCounterValue() const throw();
	/*!
	 * @abstract Current value of program-specific counter.
	 */
	float getRotationCounterValue() const throw();
	/*!
	 * @abstract Current value of turn target counter.
	 */
	float getTargetCounterValue() const throw();
	
	/*!
	 * @abstract Override counter value
	 */
	void setBlockCounterValue(float val) throw();
	/*!
	 * @abstract Override counter value
	 */
	void setRotationCounterValue(float val) throw();
	/*!
	 * @abstract Override counter value
	 */
	void setTargetCounterValue(float val) throw();	
	
	/*!
	 * @abstract Set turn target.
	 * @discussion This automatically resets the turn target counter value
	 * to zero. Set this to zero to disable turning to a specific target.
	 */
	void setTurnTarget(float value) throw();
	/*!
	 * @abstract Current target.
	 * @discussion Once the absolute turn target counter value is equal to or 
	 * larger than this, the motor is disabled.
	 */
	float getTurnTarget() const throw();
	
	/*!
	 * @abstract Set the motor’s power.
	 * @discussion This is not equal to the target speed in any way.
	 */
	void setPower(float speed) throw();
	
	/*!
	 * @abstract The motor’s power.
	 */
	float getPower() const throw();
	
	/*!
	 * @abstract The motor’s current speed.
	 * @discussion this can be 0 even if the power is not, if the turn counter
	 * goal was exceeded. it is also always in degrees/second, while power is an
	 * arbitrary unit.
	 */
	float getSpeed() const throw();
	
	/*!
	 * @abstract Is the motor running?
	 * @discussion This is true if power is > 0 and the turn limit has not been
	 * exceeded.
	 */
	bool isRunning() const throw();
};
