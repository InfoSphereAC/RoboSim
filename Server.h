/*
 *  Server.h
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 18.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>
#include <stdint.h>

#include "NetworkInterface.h"

class EnvironmentEditor;
union NetworkPacket;
class Robot;
class Simulation;

class Server : public NetworkInterface
{
	int acceptSocketv4;
	int acceptSocketv6;
	int discoverySocketv4;
	int discoverySocketv6;
		
	uint16_t portNumber;
	
	int nfds;
	
	uint32_t lastUsedClientID;
	
	Robot *localRobot;
	uint32_t clientForLocalRobot;
	Simulation *simulation;
	EnvironmentEditor *editor;
	
	bool iteratorHasReturnedLocal;
	
	bool localSensorsChangedSinceLastUpdate;
	
	struct ClientConnection
	{
		int socket;
		uint32_t clientID;
		enum {
			Connecting,
			Connected,
			Closed
		} state;
		
		Robot *robot;
		
		// Temporary storage space. There is not much buffering going on, but
		// we transmit everything that needs to be said in one burst in update()
		unsigned unsentDataLength;
		char *unsentData;
		
		// Storage for receiving
		unsigned unreadLength;
		char *unreadData;
		
        void sendPacket(NetworkPacket &packet);
		void sendData(const void *data, unsigned length);
		
		float hue;
	};
	
	std::vector<ClientConnection> clients;
	std::vector<ClientConnection>::iterator nextRobot;
	
	float lastHue;
	
	static void convertHSBtoRGB(float hueInDeg, float saturation, float brightness, float &red, float &green, float &blue);
	
	void sendAll(NetworkPacket &packet);
	
	void clientConnected(int socket);
	void readClientData(ClientConnection &connection);
	void closeClient(ClientConnection &connection);
	
	void speedUpdate(ClientConnection &client, const NetworkPacket *packet);
	void sensorUpdate(ClientConnection &client, const NetworkPacket *packet);
	void playTone(ClientConnection &client, const NetworkPacket *packet);
	void playFile(ClientConnection &client, const NetworkPacket *packet);
	void setCell(ClientConnection &client, const NetworkPacket *packet);
	void liftedMove(ClientConnection &client, const NetworkPacket *packet);
	void liftedTurn(ClientConnection &client, const NetworkPacket *packet);
	
	bool startListenIPv4();
	bool startListenIPv6();
	bool startBroadcastIPv4();
	bool startBroadcastIPv6();
	
	void receiveBroadcastPacket(int onSocket);
	
protected:
	virtual Robot *getLocalModifiableRobot() throw();
		
public:
	// Pass bitwise OR of these to constructor. Notice that the old defaults correspond to all of these!
	enum ServerCreateFlags
	{
		CreateLocalRobot = 1 << 0, // Create a local robot.
		AllowDiscovery = 1 << 1	// Allow discovery for clients, otherwise connections have to be done directly.
	};
	
	Server(unsigned short portNumber, Simulation *aSimulation, EnvironmentEditor *anEditor, unsigned flags);
	virtual ~Server();
	
	void update();
	
	// From NetworkInterface
	virtual const Robot *getLocalRobot() const throw();
	virtual void playTone(unsigned frequency, unsigned duration, bool loops, float gain);
	virtual void playFile(const char *name, bool loops, float gain);
	virtual void commitMotorChanges();
	virtual void setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range);
	virtual void setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range);
	virtual void setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument);
	virtual void updatedCellState(unsigned x, unsigned z);
	virtual Robot *getNextRobot() throw();
	virtual void setIsLifted(bool isRaised) throw();
	virtual void moveLifted(const union float4 &diff) throw();
	virtual void rotateLifted(float radian) throw();
	virtual void setRobotTurnSpeed(float turnSpeed) throw();
};
