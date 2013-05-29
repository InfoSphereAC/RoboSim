/*
 *  Client.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 06.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <map>

#include "NetworkInterface.h"

class EnvironmentEditor;
union NetworkPacket;

class Client : public NetworkInterface
{	
	int clientSocket;
	unsigned remainingUnreadCount;
	char *remainingUnread;
	
	EnvironmentEditor *editor;
	
	bool uiOnServer;
	unsigned clientID;
	
	// Keeps track of any changes to motor data since last update.
	NetworkPacket *speedChanges;
	
	// Sensor values heard from server
	bool sensorChanged;
	
	std::map<unsigned, Robot *> allRobots;
	std::map<unsigned, Robot *>::iterator nextRobot;
	
	Robot *robotForClientID(unsigned clientID) throw();
	
	void connectionAccepted(const NetworkPacket *packet);
	void robotUpdate(const NetworkPacket *packet);
	void positionUpdate(const NetworkPacket *packet);
	void sensorReadings(const NetworkPacket *packet);
	void robotDeleted(const NetworkPacket *packet);
	void playTone(const NetworkPacket *packet);
	void playFile(const NetworkPacket *packet);
	void gridOverview(const NetworkPacket *packet);
	void setCell(const NetworkPacket *packet);
	
	void send(NetworkPacket &packet);
	void send(const void *data, unsigned dataLength);
	void closeConnection();
	
	// Clears the contents of speedChanges
	void resetSpeedChanges();
	
	// From NetworkInterface
	virtual Robot *getNextRobot() throw();
	
protected:
	virtual Robot *getLocalModifiableRobot() throw();
	
public:
	Client(const struct sockaddr *networkAddress, bool ipv6, EnvironmentEditor *editor, bool uiOnServer);
	virtual ~Client();
	
	void sensorDataChanged();
	void update();
	
	// From NetworkInterface
	virtual void updatedCellState(unsigned x, unsigned z);
	
	virtual const Robot *getLocalRobot() const throw();
	virtual void playTone(unsigned frequency, unsigned duration, bool loops, float gain);
	virtual void playFile(const char *name, bool loops, float gain);
	virtual void setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range);
	virtual void setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range);
	virtual void setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument);
	virtual void setIsLifted(bool isRaised) throw();
	virtual void moveLifted(const union float4 &diff) throw();
	virtual void rotateLifted(float) throw();
	virtual void setRobotTurnSpeed(float turnSpeed) throw();
	
	// Set motor data
	virtual void commitMotorChanges(unsigned atPort) throw (std::out_of_range);
	virtual void commitTurnTargetChanges(unsigned atPort) throw (std::out_of_range);
	virtual void resetAllCounters(unsigned atPort) throw (std::out_of_range);
	virtual void resetBlockCounter(unsigned atPort) throw (std::out_of_range);
	virtual void resetRotationCounter(unsigned atPort) throw (std::out_of_range);
	virtual void setMotorTurnTarget(unsigned atPort, float target) throw (std::out_of_range);
	virtual void setMotorTurnRatio(unsigned atPort, float ratio) throw (std::out_of_range);
	virtual void setMotorForSide(unsigned port, unsigned side) throw (std::out_of_range);
	virtual void setMotorIsSynchronized(unsigned atPort, bool isit) throw (std::out_of_range);
	virtual void setMotorPower(unsigned atPort, float power) throw (std::out_of_range);	
};
