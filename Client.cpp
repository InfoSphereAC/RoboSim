/*
 *  Client.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 06.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Client.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#define ioctl ioctlsocket
#else
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#endif

#include "EnvironmentEditor.h"
#include "NetworkConstants.h"
#include "NetworkPacket.h"
#include "Robot.h"

#include <iostream>

inline Robot *Client::robotForClientID(unsigned clientID) throw()
{
	std::map<unsigned, Robot *>::iterator foundRobot = allRobots.find(clientID);
	if (foundRobot == allRobots.end()) return NULL;
	else return foundRobot->second;
}

void Client::send(NetworkPacket &packet)
{
    unsigned packetLength = packet.getNetworkLength();
    packet.swapToNetwork();
	send(&packet, packetLength);
}

void Client::send(const void *data, unsigned dataLength)
{
	if (::send(clientSocket, (char *) data, dataLength, 0) == -1)
		closeConnection();
}

void Client::connectionAccepted(const NetworkPacket *packet)
{
	// Check magic value
	if (memcmp(packet->connectionAccepted.magicValue, serverToClientHandshake, 16) != 0)
	{
		closeConnection();
		return;
	}
	
	// Read client ID
	clientID = packet->connectionAccepted.clientID;
}

void Client::robotUpdate(const NetworkPacket *packet)
{
	std::map<unsigned, Robot *>::iterator foundRobot = allRobots.find(packet->robotUpdate.clientID);
	Robot *targetRobot;
	if (foundRobot == allRobots.end())
	{
		// Create robot if it does not yet exist
		targetRobot = new Robot(NULL);
		allRobots[packet->robotUpdate.clientID] = targetRobot;
		nextRobot = allRobots.begin();
		registerNewRobot(targetRobot);
	}
	else targetRobot = foundRobot->second;
	
	for (unsigned i = 0; i < 4; i++)
	{
		targetRobot->setSensorType(i, (Robot::SensorType) packet->robotUpdate.sensor[i].type);
		targetRobot->setSensorAngle(i, packet->robotUpdate.sensor[i].angle);
		targetRobot->setIsSensorPointedDown(i, packet->robotUpdate.sensor[i].pointedDown);
		
		targetRobot->setFlagColor(packet->robotUpdate.color);
	}
}

void Client::positionUpdate(const NetworkPacket *packet)
{
	for (unsigned i = 0; i < packet->positionUpdate.numRobots; i++)
	{
		Robot *targetRobot = robotForClientID(packet->positionUpdate.robot[i].clientID);
		if (!targetRobot) continue;
		targetRobot->setPosition(packet->positionUpdate.robot[i].position);
		targetRobot->setLeftTrackSpeed(packet->positionUpdate.robot[i].trackSpeed[0]);
		targetRobot->setRightTrackSpeed(packet->positionUpdate.robot[i].trackSpeed[1]);
		targetRobot->setIsLifted(packet->positionUpdate.robot[i].isLifted != 0);
	}
}

void Client::sensorReadings(const NetworkPacket *packet)
{
	Robot *ourRobot = robotForClientID(clientID);
	if (!ourRobot) return;
	
	for (unsigned i = 0; i < 4; i++)
		ourRobot->setSensorValue(i, packet->sensorReadings.values[i]);
	
	for (unsigned i = 0; i < 3; i++)
	{
		ourRobot->getMotor(i)->setBlockCounterValue(packet->sensorReadings.motorBlockCounterValues[i]);
		ourRobot->getMotor(i)->setRotationCounterValue(packet->sensorReadings.motorRotationCounterValues[i]);
		ourRobot->getMotor(i)->setTargetCounterValue(packet->sensorReadings.motorTargetCounterValues[i]);
		ourRobot->getMotor(i)->setTurnFactor(packet->sensorReadings.turnRatio[i]);
	}
}

void Client::robotDeleted(const NetworkPacket *packet)
{
	std::map<unsigned, Robot *>::iterator deadRobot = allRobots.find(packet->robotDeleted.clientID);
	if (deadRobot == allRobots.end()) return;
	removeRobot(deadRobot->second);
	delete deadRobot->second;
	allRobots.erase(deadRobot);
}

void Client::playTone(const NetworkPacket *packet)
{
	Robot *robot = robotForClientID(packet->stcPlayTone.clientID);
	if (!robot) return;
	robot->playTone(packet->stcPlayTone.frequency, packet->stcPlayTone.duration, packet->stcPlayTone.loops, packet->stcPlayTone.volume);
}

void Client::playFile(const NetworkPacket *packet)
{
	Robot *robot = robotForClientID(packet->stcPlayTone.clientID);
	if (!robot) return;
	
	// Copy name and zero-terminate it.
	char *filename = new char [packet->stcPlayFile.nameLength + 1];
	memcpy(filename, packet->stcPlayFile.name, packet->stcPlayFile.nameLength);
	filename[packet->stcPlayFile.nameLength] = '\0';
	
	robot->playFile(filename, packet->stcPlayFile.loops, packet->stcPlayFile.volume);
	
	delete filename;
}

void Client::gridOverview(const NetworkPacket *packet)
{
	editor->loadFromSerialization(packet);
}

void Client::setCell(const NetworkPacket *packet)
{
	try
	{
		editor->setCellIsWall(packet->setCell.x, packet->setCell.z, packet->setCell.cell & 0x80);
		editor->setCellShade(packet->setCell.x, packet->setCell.z, float(packet->setCell.cell & 0x7F) / 127.0f);
	}
	catch (std::runtime_error e)
	{
		closeConnection();
	}
}

void Client::closeConnection()
{
	shutdown(clientSocket, 2);
#ifdef _WIN32
	closesocket(clientSocket);
#else
	close(clientSocket);
#endif
	clientSocket = 0;
}

Client::Client(const struct sockaddr *networkAddress, bool ipv6, EnvironmentEditor *anEditor, bool putUIOnServer)
: editor(anEditor)
{
	clientSocket = socket(ipv6 ? PF_INET6 : PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == -1)
		throw("Could not open socket.");
	if (connect(clientSocket, networkAddress, ipv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in)) == -1)
	{
		fprintf(stderr, "Can’t connect: %u", errno);
		throw std::runtime_error("Could not connect.");
	}
	
	uiOnServer = putUIOnServer;
	
	NetworkPacket connectionRequestPacket;
	connectionRequestPacket.packetType = NetworkPacket::ConnectionRequest;
	connectionRequestPacket.packetLength = sizeof(ConnectionRequestPacket);
	connectionRequestPacket.connectionRequest.versionNumber = protocolVersionNumber;
	memcpy(connectionRequestPacket.connectionRequest.magicValue, clientToServerHandshake, 16);
	connectionRequestPacket.connectionRequest.flags = putUIOnServer ? NetworkControlledByServer : 0;
	send(connectionRequestPacket);
	
	nextRobot = allRobots.begin();
	sensorChanged = false;
	remainingUnread = 0;
	remainingUnreadCount = 0;
	
	// Set up empty packet for speed changes
	speedChanges = new NetworkPacket;
	speedChanges->packetType = NetworkPacket::SpeedUpdate;
	speedChanges->packetLength = sizeof(SpeedUpdatePacket);
	speedChanges->speedUpdate.flags = 0;
	speedChanges->speedUpdate.isMotorSynchronized = 0;
	speedChanges->speedUpdate.motors[0].flags = 0;
	speedChanges->speedUpdate.motors[0].power = 0.0f;
	speedChanges->speedUpdate.motors[0].turnTarget = 0.0f;
	speedChanges->speedUpdate.motors[0].turnRatio = 0.0f;
	speedChanges->speedUpdate.motors[1].flags = 0;
	speedChanges->speedUpdate.motors[1].power = 0.0f;
	speedChanges->speedUpdate.motors[1].turnTarget = 0.0f;
	speedChanges->speedUpdate.motors[1].turnRatio = 0.0f;
	speedChanges->speedUpdate.motors[2].flags = 0;
	speedChanges->speedUpdate.motors[2].power = 0.0f;
	speedChanges->speedUpdate.motors[2].turnTarget = 0.0f;
	speedChanges->speedUpdate.motors[2].turnRatio = 0.0f;
}

Client::~Client()
{
	for (std::map<unsigned, Robot *>::iterator iter = allRobots.begin(); iter != allRobots.end(); ++iter)
	{
		removeRobot(iter->second);
		delete iter->second;
	}
	closeConnection();

	delete [] remainingUnread;
	delete speedChanges;
}

void Client::update()
{
	if (!clientSocket) return;
	
	// Get new data from server, if any.
	while(true)
	{
		fd_set clientSocketSet;
		FD_ZERO(&clientSocketSet);
		FD_SET(clientSocket, &clientSocketSet);
	
		struct timeval immediately = {0, 0};
		int selectResult = select(clientSocket + 1, &clientSocketSet, NULL, NULL, &immediately);
	
		if (selectResult < 0) throw("Error in select!");
		if (selectResult == 0 || !FD_ISSET(clientSocket, &clientSocketSet)) break;

		if (clientSocket == -1) return;
			
		// Read data from server
#ifdef _WIN32
		unsigned long numBytesToRead;
#else
		int numBytesToRead;
#endif
		if (ioctl(clientSocket, FIONREAD, &numBytesToRead) != 0)
		{
			closeConnection();
			return;
		}

		char *data = new char [numBytesToRead + remainingUnreadCount];
		memcpy(data, remainingUnread, remainingUnreadCount);
		unsigned actuallyRead = recv(clientSocket, &(data[remainingUnreadCount]), numBytesToRead, 0);
		if (actuallyRead == 0)
		{
			closeConnection();
			return;
		}
		unsigned totalData = actuallyRead + remainingUnreadCount;

		NetworkPacket *packet = reinterpret_cast<NetworkPacket *> (data);
		unsigned offsetSoFar = 0;
			
		delete [] remainingUnread;
		remainingUnread = 0;
		remainingUnreadCount = 0;
			
		while (offsetSoFar < (unsigned) totalData)
		{
			unsigned currentUnreadCount = (totalData - offsetSoFar);
			if (!packet->swappedFitsWithinRemainingBytes(currentUnreadCount))
			{
				remainingUnreadCount = currentUnreadCount;
				remainingUnread = new char [remainingUnreadCount];
				memcpy(remainingUnread, &(data[offsetSoFar]), remainingUnreadCount);
				break;
			}
            
            packet->swapFromNetwork();
			switch (packet->packetType)
			{
				case NetworkPacket::ConnectionAccepted:
					connectionAccepted(packet);
					break;
				case NetworkPacket::RobotUpdate:
					robotUpdate(packet);
					break;
				case NetworkPacket::PositionUpdate:
					positionUpdate(packet);
					break;
				case NetworkPacket::SensorReadings:
					sensorReadings(packet);
					break;
				case NetworkPacket::RobotDeleted:
					robotDeleted(packet);
					break;
				case NetworkPacket::StCPlayTone:
					playTone(packet);
					break;
				case NetworkPacket::StCPlayFile:
					playFile(packet);
					break;
				case NetworkPacket::GridOverview:
					gridOverview(packet);
					break;
				case NetworkPacket::SetCell:
					setCell(packet);
					break;
						
				default:
					closeConnection();
					delete [] data;
					return;
			}
				
			// Read next packet
			offsetSoFar += packet->getNetworkLength();
			packet = reinterpret_cast<NetworkPacket *> (&(data[offsetSoFar]));
		}
			
		delete [] data;
	}
	
	// Send data to server
	if (speedChanges->speedUpdate.flags || speedChanges->speedUpdate.motors[0].flags || speedChanges->speedUpdate.motors[1].flags || speedChanges->speedUpdate.motors[2].flags)
	{
		send(*speedChanges);
		speedChanges->speedUpdate.flags = 0;
		speedChanges->speedUpdate.isMotorSynchronized = 0;
		speedChanges->speedUpdate.motors[0].flags = 0;
		speedChanges->speedUpdate.motors[1].flags = 0;
		speedChanges->speedUpdate.motors[2].flags = 0;
	}
	
	if (sensorChanged)
	{
		NetworkPacket sensorChangedPacket;
		sensorChangedPacket.packetType = NetworkPacket::SensorUpdate;
		sensorChangedPacket.packetLength = sizeof(SensorUpdatePacket);
		sensorChangedPacket.sensorUpdate.leftMotorIndex = getLocalRobot()->getPortForLeftMotor();
		sensorChangedPacket.sensorUpdate.rightMotorIndex = getLocalRobot()->getPortForRightMotor();
		for (unsigned i = 0; i < 4; i++)
		{
			sensorChangedPacket.sensorUpdate.sensor[i].type = uint32_t(getLocalRobot()->getSensorType(i));
			sensorChangedPacket.sensorUpdate.sensor[i].angle = getLocalRobot()->getSensorAngle(i);
			sensorChangedPacket.sensorUpdate.sensor[i].pointedDown = uint8_t(getLocalRobot()->isSensorPointedDown(i));
		}
		
		send(sensorChangedPacket);
		
		sensorChanged = false;
	}
}

// Robot Network interface
void Client::playTone(unsigned frequency, unsigned duration, bool loops, float gain)
{
	NetworkPacket playTonePacket;
	playTonePacket.packetType = NetworkPacket::CtSPlayTone;
	playTonePacket.packetLength = sizeof(CtSPlayTonePacket);
	playTonePacket.ctsPlayTone.frequency = frequency;
	playTonePacket.ctsPlayTone.duration = duration;
	playTonePacket.ctsPlayTone.loops = loops;
	playTonePacket.ctsPlayTone.volume = gain;
	
	send(playTonePacket);
}

void Client::playFile(const char *name, bool loops, float gain)
{	
	NetworkPacket *playFilePacket = reinterpret_cast<NetworkPacket *>(malloc(sizeof(CtSPlayFilePacket) + strlen(name)));
	playFilePacket->ctsPlayFile.packetType = NetworkPacket::CtSPlayFile;
	playFilePacket->ctsPlayFile.packetLength = sizeof(CtSPlayFilePacket) + strlen(name);
	playFilePacket->ctsPlayFile.loops = loops;
	playFilePacket->ctsPlayFile.volume = gain;
	playFilePacket->ctsPlayFile.nameLength = strlen(name);
	
	// It is intentional that the trailing zero is not transmitted. We have the
	// nameLength field for that information.
	memcpy(playFilePacket->ctsPlayFile.name, name, strlen(name));
	
	send(*playFilePacket);
	free(playFilePacket);
}

void Client::updatedCellState(unsigned x, unsigned z)
{
	bool isWall = editor->getCellIsWall(x, z);
	float color = editor->getCellShade(x, z);
	
	NetworkPacket cellUpdatePacket;
	cellUpdatePacket.packetType = NetworkPacket::SetCell;
	cellUpdatePacket.packetLength = sizeof(SetCellPacket);
	cellUpdatePacket.setCell.x = x;
	cellUpdatePacket.setCell.z = z;
	cellUpdatePacket.setCell.cell = (isWall ? 0x80 : 0x00) + uint8_t(color * 127.0f);
	
	send(cellUpdatePacket);
}

void Client::setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range)
{
	Robot *localRobot = robotForClientID(clientID);
	if (!localRobot) return;
	
	localRobot->setSensorAngle(sensor, angleInDegrees);
	sensorChanged = true;
}

void Client::setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range)
{
	Robot *localRobot = robotForClientID(clientID);
	if (!localRobot) return;
	
	localRobot->setIsSensorPointedDown(sensor, isit);
	sensorChanged = true;
}

void Client::setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument)
{
	Robot *localRobot = robotForClientID(clientID);
	if (!localRobot) return;
	
	localRobot->setSensorType(sensor, type);
	sensorChanged = true;
}

void Client::setIsLifted(bool isRaised) throw()
{
	NetworkPacket liftedPacket;
	liftedPacket.packetType = NetworkPacket::LiftedMove;
	liftedPacket.packetLength = sizeof(LiftedMovePacket);
	liftedPacket.liftedMove.isLifted = isRaised;
	liftedPacket.liftedMove.delta[0] = 0.0f;
	liftedPacket.liftedMove.delta[1] = 0.0f;
	liftedPacket.liftedMove.delta[2] = 0.0f;
	
	send(liftedPacket);
}

void Client::moveLifted(const union float4 &diff) throw()
{
	NetworkPacket liftedPacket;
	liftedPacket.packetType = NetworkPacket::LiftedMove;
	liftedPacket.packetLength = sizeof(LiftedMovePacket);
	liftedPacket.liftedMove.isLifted = true;
	liftedPacket.liftedMove.delta[0] = diff.x;
	liftedPacket.liftedMove.delta[1] = diff.y;
	liftedPacket.liftedMove.delta[2] = diff.z;
	
	send(liftedPacket);
}

void Client::rotateLifted(float radians) throw()
{
	NetworkPacket liftedTurnPacket;
	liftedTurnPacket.packetType = NetworkPacket::LiftedTurn;
	liftedTurnPacket.packetLength = sizeof(LiftedTurnPacket);
	liftedTurnPacket.liftedTurn.turnSpeed = 0.0f;
	liftedTurnPacket.liftedTurn.turnDirectly = radians;
	
	send(liftedTurnPacket);
}

void Client::setRobotTurnSpeed(float turnSpeed) throw()
{
	NetworkPacket liftedTurnPacket;
	liftedTurnPacket.packetType = NetworkPacket::LiftedTurn;
	liftedTurnPacket.packetLength = sizeof(LiftedTurnPacket);
	liftedTurnPacket.liftedTurn.turnSpeed = turnSpeed;
	liftedTurnPacket.liftedTurn.turnDirectly = 0.0f;
	
	send(liftedTurnPacket);
}

Robot *Client::getLocalModifiableRobot() throw()
{
	return robotForClientID(clientID);
}

const Robot *Client::getLocalRobot() const throw()
{
	// Not using robotForClientID because that is not const.
	std::map<unsigned, Robot *>::const_iterator foundRobot = allRobots.find(clientID);
	if (foundRobot == allRobots.end()) return NULL;
	else return foundRobot->second;	
}

Robot *Client::getNextRobot() throw()
{
	if (nextRobot == allRobots.end())
	{
		nextRobot = allRobots.begin();
		return 0;
	}
	else
	{
		Robot *result = nextRobot->second;
		++nextRobot;
		return result;
	}
}

void Client::commitMotorChanges(unsigned atPort) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkSetPower;
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkSetTurnRatio;
	// Do not set on robot, gets set by server
}

void Client::commitTurnTargetChanges(unsigned atPort) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkSetTurnTarget;
	// Do not set on robot, gets set by server
}

void Client::resetAllCounters(unsigned atPort) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkResetAllCounters;
	// Do not set on robot, gets set by server
}

void Client::resetBlockCounter(unsigned atPort) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkResetBlockCounter;
	// Do not set on robot, gets set by server
}

void Client::resetRotationCounter(unsigned atPort) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].flags |= NetworkResetRotationCounter;
	// Do not set on robot, gets set by server
}

void Client::setMotorTurnTarget(unsigned atPort, float target) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].turnTarget = target;
	
	// Set state on robot as well, for getting
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot)
		localRobot->getMotor(atPort)->setTurnTarget(target);
}

void Client::setMotorTurnRatio(unsigned atPort, float ratio) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.motors[atPort].turnRatio = ratio;
	// Do not set on robot, gets set by server
}

void Client::setMotorIsSynchronized(unsigned atPort, bool isit) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	speedChanges->speedUpdate.flags |= (1 << atPort);
	
	speedChanges->speedUpdate.isMotorSynchronized &= ~(1 << atPort); // Clear bit
	speedChanges->speedUpdate.isMotorSynchronized |= (isit << atPort); // Reset it (or not)
	
	// Set state on robot as well, for getting
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot)
		localRobot->setMotorIsSynchronized(atPort, isit);
}

void Client::setMotorPower(unsigned atPort, float power) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
		
	speedChanges->speedUpdate.motors[atPort].power = power;
	
	// Set state on robot as well, for getting
	Robot *localRobot = getLocalModifiableRobot();
	if (localRobot)
		localRobot->getMotor(atPort)->setPower(power);
}

void Client::setMotorForSide(unsigned atPort, unsigned side) throw (std::out_of_range)
{
	if (atPort >= 3) throw std::out_of_range("Port out of range");
	
	sensorChanged = true;
	
	// Set state on robot as well, for getting
	Robot *localRobot = getLocalModifiableRobot();
	if (!localRobot) return;
	if (side == 0) localRobot->setPortForLeftMotor(atPort);
	else if (side == 1) localRobot->setPortForRightMotor(atPort);
}

