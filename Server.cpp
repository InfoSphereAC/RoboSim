/*
 *  Server.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 18.05.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "Server.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define ioctl ioctlsocket
#else
#include <arpa/inet.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef ANDROID_NDK
#include <netinet/in6.h>
#endif /* ANDROID_NDK */
#endif /* _WIN32 */

#include "EnvironmentEditor.h"
#include "NetworkConstants.h"
#include "NetworkPacket.h"
#include "Robot.h"
#include "Simulation.h"

#include <iostream>

namespace
{
	const float hueDifference = 70.0f;
}

void Server::convertHSBtoRGB(float hueInDeg, float saturation, float brightness, float &red, float &green, float &blue)
{
	float h = std::floor(hueInDeg/60);
	float f = (hueInDeg/60 - h);
	
	float p = brightness * (1 - saturation);
	float q = brightness * (1 - saturation * f);
	float t = brightness * (1 - saturation * (1 - f));
	
	if (h == 0 || h == 6)
	{
		red = brightness;
		green = t;
		blue = p;
	}
	else if (h == 1)
	{
		red = q;
		green = brightness;
		blue = p;
	}
	else if (h == 2)
	{
		red = p;
		green = brightness;
		blue = t;
	}
	else if (h == 3)
	{
		red = p;
		green = q;
		blue = brightness;
	}
	else if (h == 4)
	{
		red = t;
		green = p;
		blue = brightness;
	}
	else if (h == 5)
	{
		red = brightness;
		green = p;
		blue = q;
	}
}

void Server::sendAll(NetworkPacket &packet)
{
    unsigned packetLength = packet.getNetworkLength();
    packet.swapToNetwork();
	for (std::vector<ClientConnection>::iterator client = clients.begin(); client != clients.end(); ++client)
		if (client->state != ClientConnection::Closed)
			client->sendData(&packet, packetLength);
}

void Server::ClientConnection::sendPacket(NetworkPacket &packet)
{
    unsigned packetLength = packet.getNetworkLength();
    packet.swapToNetwork();
    sendData(&packet, packetLength);
}

void Server::ClientConnection::sendData(const void *data, unsigned length)
{
	unsentData = reinterpret_cast<char *> (realloc(unsentData, unsentDataLength + length));
	memcpy(&unsentData[unsentDataLength], data, length);
	unsentDataLength += length;
}

void Server::clientConnected(int connectingSocket)
{
	
	struct sockaddr address;
	socklen_t address_len = sizeof(address);
	
	int newSocket = accept(connectingSocket, &address, &address_len);
	if (newSocket == -1)
	{
		throw("Could not accept.");
	}
	
	// Add to client list
	
	ClientConnection connection;
	connection.socket = newSocket;
	connection.state = ClientConnection::Connecting;
	connection.clientID = lastUsedClientID + 1;
	lastUsedClientID = connection.clientID;
	connection.robot = 0;
	connection.unsentDataLength = 0;
	connection.unsentData = 0;
	connection.unreadLength = 0;
	connection.unreadData = 0;
	clients.push_back(connection);
	
	// Update nfds
	if (newSocket >= nfds) nfds = newSocket + 1;
}

void Server::closeClient(ClientConnection &client)
{
	if (client.robot)
	{
		if (client.robot == localRobot) localRobot = 0;
		
		simulation->removeRobot(client.robot);
		removeRobot(client.robot);
		delete client.robot;
	}
	if (client.unsentData) free(client.unsentData);
	if (client.socket)
	{
		shutdown(client.socket, 2);
#ifdef _WIN32
		closesocket(client.socket);
#else
		close(client.socket);
#endif
	}
	if (client.unreadData) delete [] client.unreadData;
	
	client.unsentData = 0;
	client.robot = 0;
	client.socket = 0;
	client.unsentDataLength = 0;
	client.unreadData = 0;
	client.unreadLength = 0;
	client.state = ClientConnection::Closed;
	
	
	NetworkPacket deleteRobotPacket;
	deleteRobotPacket.robotDeleted.packetType = NetworkPacket::RobotDeleted;
	deleteRobotPacket.robotDeleted.packetLength = sizeof(RobotDeletedPacket);
	deleteRobotPacket.robotDeleted.clientID = client.clientID;
	sendAll(deleteRobotPacket);
}

void Server::speedUpdate(ClientConnection &client, const NetworkPacket *packet)
{
	if (!client.robot) return;
		
	if (packet->speedUpdate.flags & NetworkChangeMotorASynchronized)
		client.robot->setMotorIsSynchronized(0, packet->speedUpdate.isMotorSynchronized & (1 << 0));
	if (packet->speedUpdate.flags & NetworkChangeMotorBSynchronized)
		client.robot->setMotorIsSynchronized(1, packet->speedUpdate.isMotorSynchronized & (1 << 1));
	if (packet->speedUpdate.flags & NetworkChangeMotorCSynchronized)
		client.robot->setMotorIsSynchronized(2, packet->speedUpdate.isMotorSynchronized & (1 << 2));
	
	for (unsigned i = 0; i < 3; i++)
	{
		if (packet->speedUpdate.motors[i].flags & NetworkSetPower)
			client.robot->getMotor(i)->setPower(packet->speedUpdate.motors[i].power);
		if (packet->speedUpdate.motors[i].flags & NetworkSetTurnTarget)
			client.robot->getMotor(i)->setTurnTarget(packet->speedUpdate.motors[i].turnTarget);
		if (packet->speedUpdate.motors[i].flags & NetworkSetTurnRatio)
			client.robot->getMotor(i)->setTurnFactor(packet->speedUpdate.motors[i].turnRatio);
		
		if (packet->speedUpdate.motors[i].flags & NetworkResetBlockCounter)
			client.robot->getMotor(i)->resetBlockCounter();
		if (packet->speedUpdate.motors[i].flags & NetworkResetRotationCounter)
			client.robot->getMotor(i)->resetRotationCounter();
		if (packet->speedUpdate.motors[i].flags & NetworkResetAllCounters)
			client.robot->getMotor(i)->resetAllCounters();
	}
}

void Server::sensorUpdate(ClientConnection &client, const NetworkPacket *packet)
{
	if (client.robot == localRobot) return; // Ignore if sent from a client who told us to manage the robot.
	
	if (packet->sensorUpdate.leftMotorIndex != 255)
		client.robot->setPortForLeftMotor(packet->sensorUpdate.leftMotorIndex);
	
	if (packet->sensorUpdate.rightMotorIndex != 255)
		client.robot->setPortForRightMotor(packet->sensorUpdate.rightMotorIndex);	
	
	// Change the sensor state
	for (unsigned i = 0; i < 4; i++)
	{
		client.robot->setSensorType(i, (Robot::SensorType) packet->sensorUpdate.sensor[i].type);
		client.robot->setSensorAngle(i, packet->sensorUpdate.sensor[i].angle);
		client.robot->setIsSensorPointedDown(i, packet->sensorUpdate.sensor[i].pointedDown);
	}
	
	// Transmit this to everyone else as well
	NetworkPacket robotUpdatePacket;
	robotUpdatePacket.packetType = NetworkPacket::RobotUpdate;
	robotUpdatePacket.packetLength = sizeof(RobotUpdatePacket);
	robotUpdatePacket.robotUpdate.clientID = client.clientID;
	
	// Copy sensor data directly over
	memcpy(&(robotUpdatePacket.robotUpdate.sensor), &(packet->sensorUpdate.sensor), sizeof(packet->sensorUpdate.sensor));
	
	// Copy color
	memcpy(robotUpdatePacket.robotUpdate.color, client.robot->getFlagColor(), sizeof(float [3]));
	
	sendAll(robotUpdatePacket);	
}

void Server::playTone(ClientConnection &client, const NetworkPacket *packet)
{
	if (!client.robot) return;
	
	client.robot->playTone(packet->ctsPlayTone.frequency, packet->ctsPlayTone.duration, packet->ctsPlayTone.loops, packet->ctsPlayTone.volume);
	
	NetworkPacket playSoundPacket;
	playSoundPacket.packetType = NetworkPacket::StCPlayTone;
	playSoundPacket.packetLength = sizeof(StCPlayTonePacket);
	playSoundPacket.stcPlayTone.clientID = client.clientID;
	playSoundPacket.stcPlayTone.frequency = packet->ctsPlayTone.frequency;
	playSoundPacket.stcPlayTone.loops = packet->ctsPlayTone.loops;
	playSoundPacket.stcPlayTone.volume = packet->ctsPlayTone.volume;
	
	sendAll(playSoundPacket);
}

void Server::playFile(ClientConnection &client, const NetworkPacket *packet)
{
	if (!client.robot) return;

	// Copy filename and make local robot play the sound
	char *filename = new char [packet->ctsPlayFile.nameLength + 1];
	memcpy(filename, packet->ctsPlayFile.name, packet->ctsPlayFile.nameLength);
	filename[packet->ctsPlayFile.nameLength] = '\0';
	client.robot->playFile(filename, packet->ctsPlayFile.loops, packet->ctsPlayFile.volume);	
	delete filename;
	
	// Also, send information to all others
	NetworkPacket *playFilePacket = reinterpret_cast<NetworkPacket *>(malloc(sizeof(StCPlayFilePacket) + packet->ctsPlayFile.nameLength));
	playFilePacket->stcPlayFile.packetType = NetworkPacket::StCPlayFile;
	playFilePacket->stcPlayFile.packetLength = sizeof(StCPlayFilePacket) + packet->ctsPlayFile.nameLength;
	playFilePacket->stcPlayFile.clientID = client.clientID;
	playFilePacket->stcPlayFile.loops = packet->ctsPlayFile.loops;
	playFilePacket->stcPlayFile.volume = packet->ctsPlayFile.volume;
	playFilePacket->stcPlayFile.nameLength = packet->ctsPlayFile.nameLength;
	
	// It is intentional that the trailing zero is not transmitted. We have the
	// nameLength field for that information.
	memcpy(playFilePacket->stcPlayFile.name, packet->ctsPlayFile.name, packet->ctsPlayFile.nameLength);
	
	sendAll(*playFilePacket);	
	free(playFilePacket);
}

void Server::setCell(ClientConnection &client, const NetworkPacket *packet)
{
	try
	{
		editor->setCellIsWall(packet->setCell.x, packet->setCell.z, (packet->setCell.cell & 0x80) != 0);
		editor->setCellShade(packet->setCell.x, packet->setCell.z, float(packet->setCell.cell & 0x7F) / 127.0f);
        NetworkPacket backPacket;
        memcpy(&backPacket, packet, packet->getNetworkLength());
		sendAll(backPacket);
	}
	catch (std::runtime_error e)
	{
		closeClient(client);
	}
}

void Server::liftedMove(ClientConnection &client, const NetworkPacket *packet)
{
	if (client.robot == localRobot) return; // Ignore if sent from a client who told us to manage the robot.
	client.robot->setIsLifted(packet->liftedMove.isLifted);
	client.robot->moveDirectly(float4(packet->liftedMove.delta[0], packet->liftedMove.delta[1], packet->liftedMove.delta[2], 0.0f));
}
void Server::liftedTurn(ClientConnection &client, const NetworkPacket *packet)
{
	if (client.robot == localRobot) return; // Ignore if sent from a client who told us to manage the robot.
	client.robot->setLiftedTurnSpeed(packet->liftedTurn.turnSpeed);
	client.robot->rotate(packet->liftedTurn.turnDirectly);
}

void Server::readClientData(ClientConnection &client)
{
#ifdef _WIN32
	unsigned long numBytesToRead;
#else
	int numBytesToRead;
#endif

	if (ioctl(client.socket, FIONREAD, &numBytesToRead) != 0)
		throw std::runtime_error("Could not get bytes to read.");
	
	if (numBytesToRead == 0)
	{
		// Available for read, but no data, means a close according to the
		// rules of select.
		closeClient(client);
		return;
	}
	
	char *data = new char [numBytesToRead + client.unreadLength];
	memcpy(data, client.unreadData, client.unreadLength);
	unsigned actuallyRead = recv(client.socket, &(data[client.unreadLength]), numBytesToRead, 0);
	if (actuallyRead == 0)
	{
		delete [] data;
		closeClient(client);
		return;
	}
	
	unsigned totalBytes = actuallyRead + client.unreadLength;
	delete client.unreadData;
	client.unreadData = 0;
	client.unreadLength = 0;
	
	NetworkPacket *packet = reinterpret_cast<NetworkPacket *> (data);
	unsigned offsetSoFar = 0;
	
	while (offsetSoFar < totalBytes)
	{
		unsigned currentUnreadCount = (totalBytes - offsetSoFar);		
		if (!packet->swappedFitsWithinRemainingBytes(currentUnreadCount))
		{
			client.unreadLength = currentUnreadCount;
			client.unreadData = new char [client.unreadLength];
			memcpy(client.unreadData, &(data[offsetSoFar]), client.unreadLength);
			break;
		}
        packet->swapFromNetwork();
		
		if (client.state == ClientConnection::Connecting)
		{
			// First transmission has to be a connection request
			if (packet->packetType != NetworkPacket::ConnectionRequest)
			{
				closeClient(client);
				delete [] data;
				return;
			}
			
			// Check for protocol version
			if (packet->connectionRequest.versionNumber != protocolVersionNumber)
			{
				closeClient(client);
				delete [] data;
				return;
			}
			
			// Check for magic value
			if (memcmp(packet->connectionRequest.magicValue, clientToServerHandshake, 16) != 0)
			{
				closeClient(client);
				delete [] data;
				return;
			}
			
			// Check whether it requests its robot to be set as the local robot
			if (packet->connectionRequest.flags & NetworkControlledByServer)
			{
				if (localRobot) // Check that no local robot exists yet.
				{
					closeClient(client);
					delete [] data;
					return;
				}
			}
			
			// Client seems allright. Transmit reply.
			NetworkPacket connectionAcceptedPacket;
			connectionAcceptedPacket.packetType = NetworkPacket::ConnectionAccepted;
			connectionAcceptedPacket.packetLength = sizeof(ConnectionAcceptedPacket);
			connectionAcceptedPacket.connectionAccepted.clientID = clients.back().clientID;
			memcpy(connectionAcceptedPacket.connectionAccepted.magicValue, serverToClientHandshake, 16);
			client.sendPacket(connectionAcceptedPacket);
			
			// Transmit the current environment
			NetworkPacket *environmentPacket = editor->writeToSerialization();
			
			client.sendPacket(*environmentPacket);
			free(environmentPacket);
			
			// Transmit our own robot
			if (localRobot && (clientForLocalRobot != 0))
			{
				NetworkPacket serverRobotPacket;
				serverRobotPacket.packetType = NetworkPacket::RobotUpdate;
				serverRobotPacket.packetLength = sizeof(RobotUpdatePacket);
				serverRobotPacket.robotUpdate.clientID = 0;
				float red, blue, green;
				convertHSBtoRGB(0.0f, 1.0f, 1.0f, red, blue, green);
				serverRobotPacket.robotUpdate.color[0] = red;
				serverRobotPacket.robotUpdate.color[1] = blue;
				serverRobotPacket.robotUpdate.color[2] = green;
				for (unsigned i = 0; i < 4; i++)
				{
					serverRobotPacket.robotUpdate.sensor[i].type = localRobot->getSensorType(i);
					serverRobotPacket.robotUpdate.sensor[i].angle = localRobot->getSensorAngle(i);
					serverRobotPacket.robotUpdate.sensor[i].pointedDown = localRobot->isSensorPointedDown(i);
				}
				client.sendPacket(serverRobotPacket);
			}
			
			// Create new robot for that client
			client.robot = new Robot(simulation);
			float red, blue, green;
			convertHSBtoRGB(lastHue, 1.0f, 1.0f, red, blue, green);
			lastHue = std::fmod(lastHue + hueDifference, 360);
			float color[] = {red, blue, green};
			client.robot->setFlagColor(color);
			registerNewRobot(client.robot);
			simulation->addRobot(client.robot);
			if (packet->connectionRequest.flags & NetworkControlledByServer)
			{
				localRobot = client.robot;
				clientForLocalRobot = client.clientID;
			}
			
			// Reset that list
			nextRobot = clients.begin();	
			
			// Transmit other robots
			for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
			{
				NetworkPacket robotPacket;
				robotPacket.packetType = NetworkPacket::RobotUpdate;
				robotPacket.packetLength = sizeof(RobotUpdatePacket);
				robotPacket.robotUpdate.clientID = iter->clientID;
				for (unsigned i = 0; i < 4; i++)
				{
					robotPacket.robotUpdate.sensor[i].type = iter->robot->getSensorType(i);
					robotPacket.robotUpdate.sensor[i].angle = iter->robot->getSensorAngle(i);
					robotPacket.robotUpdate.sensor[i].pointedDown = iter->robot->isSensorPointedDown(i);
					memcpy(robotPacket.robotUpdate.color, iter->robot->getFlagColor(), sizeof(float [3]));
				}
				if (iter->clientID == client.clientID)
				{
					// The new robot, tell everyone about it
					sendAll(robotPacket);
				}
				else
					client.sendPacket(robotPacket);
			}
			
			// Set state
			client.state = ClientConnection::Connected;
			
		}
		else if (client.state == ClientConnection::Connected)
		{
			switch (packet->packetType)
			{
				case NetworkPacket::SpeedUpdate:
					speedUpdate(client, packet);
					break;
				case NetworkPacket::SensorUpdate:
					sensorUpdate(client, packet);
					break;
				case NetworkPacket::CtSPlayTone:
					playTone(client, packet);
					break;
				case NetworkPacket::CtSPlayFile:
					playFile(client, packet);
					break;
				case NetworkPacket::SetCell:
					setCell(client, packet);
					break;
				case NetworkPacket::LiftedMove:
					liftedMove(client, packet);
					break;
				case NetworkPacket::LiftedTurn:
					liftedTurn(client, packet);
					break;
					
				default: // Unknown packet type
					closeClient(client);
			}
		}
		
		// Skip to next packet, or end if at end.
		offsetSoFar += packet->getNetworkLength();
		if (offsetSoFar >= (unsigned) numBytesToRead)
		{
			delete [] data;
			return;
		}
		packet = reinterpret_cast<NetworkPacket *> (&(data[offsetSoFar]));
	}
}

bool Server::startListenIPv4()
{
	acceptSocketv4 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (acceptSocketv4 == -1) return false;
	
	struct sockaddr_in anyV4;
#ifdef __APPLE_CC__
	anyV4.sin_len = sizeof(anyV4);
#endif
	anyV4.sin_family = AF_INET;
	anyV4.sin_addr.s_addr = htonl(INADDR_ANY);
	anyV4.sin_port = htons(portNumber);
	
	if (bind(acceptSocketv4, reinterpret_cast<const struct sockaddr *>(&anyV4), sizeof(anyV4)) < 0)
	{
		std::cout << "sever: could not bind acceptsocket 4" << std::endl;
#ifdef _WIN32
		closesocket(acceptSocketv4);
#else
		close(acceptSocketv4);
#endif
		acceptSocketv4 = -1;
		return false;
	}
	if (listen(acceptSocketv4, maxBacklog) < 0)
	{
		std::cout << "could not listen with acceptsocket 4" << std::endl;
#ifdef _WIN32
		closesocket(acceptSocketv4);
#else
		close(acceptSocketv4);
#endif
		acceptSocketv4 = -1;
		return false;
	}
	
	if (acceptSocketv4 >= nfds) nfds = acceptSocketv4 + 1;
	
	return true;
}

bool Server::startListenIPv6()
{
	acceptSocketv6 = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (acceptSocketv6 == -1) return false;
	
	struct sockaddr_in6 anyV6;
#ifdef __APPLE_CC__
	anyV6.sin6_len = sizeof(anyV6);
#endif
	anyV6.sin6_family = AF_INET6;
	anyV6.sin6_addr = in6addr_any;
	anyV6.sin6_port = htons(portNumber);
	
	if (bind(acceptSocketv6, reinterpret_cast<const struct sockaddr *>(&anyV6), sizeof(anyV6)) < 0)
	{
		std::cout << "could not bind acceptsocket 6" << std::endl;
#ifdef _WIN32
		closesocket(acceptSocketv6);
#else
		close(acceptSocketv6);
#endif
		acceptSocketv6 = -1;
		return false;
	}
	if (listen(acceptSocketv6, maxBacklog) < 0)
	{
		std::cout << "could not listen with acceptsocket 6" << std::endl;
#ifdef _WIN32
		closesocket(acceptSocketv6);
#else
		close(acceptSocketv6);
#endif
		acceptSocketv6 = -1;
		return false;
	}
	
	if (acceptSocketv6 >= nfds) nfds = acceptSocketv6 + 1;
	
	return true;
}

bool Server::startBroadcastIPv4()
{
	discoverySocketv4 = socket(PF_INET, SOCK_DGRAM, 0);
	if (discoverySocketv4 == -1) return false;
	
	int socketOptionValue = 1;
	if (setsockopt(discoverySocketv4, SOL_SOCKET, SO_BROADCAST, (const char *) &socketOptionValue, sizeof(socketOptionValue)) == -1)
	{
		std::cout << "Could not configure socket for broadcast (6)" << std::endl;
#ifdef _WIN32
		closesocket(discoverySocketv4);
#else
		close(discoverySocketv4);
#endif
		discoverySocketv4 = -1;
		return false;
	}	
	
	struct sockaddr_in anyV4;
#ifdef __APPLE_CC__
	anyV4.sin_len = sizeof(anyV4);
#endif
	anyV4.sin_family = AF_INET;
	anyV4.sin_addr.s_addr = htonl(INADDR_ANY);
	anyV4.sin_port = htons(portNumber);
	
	if (bind(discoverySocketv4, reinterpret_cast<const struct sockaddr *>(&anyV4), sizeof(anyV4)) < 0)
	{
		std::cout << "server: could not bind broadcast socket 4" << std::endl;
#ifdef _WIN32
		closesocket(discoverySocketv4);
#else
		close(discoverySocketv4);
#endif
		discoverySocketv4 = -1;
		return false;
	}
	
	if (discoverySocketv4 >= nfds) nfds = discoverySocketv4 + 1;
	
	return true;
}

bool Server::startBroadcastIPv6()
{
	discoverySocketv6 = socket(PF_INET6, SOCK_DGRAM, 0);
	if (discoverySocketv6 == -1) return false;
	
	int socketOptionValue = 1;
	if (setsockopt(discoverySocketv6, SOL_SOCKET, SO_BROADCAST, (const char *) &socketOptionValue, sizeof(socketOptionValue)) == -1)
	{
		std::cout << "Could not configure socket for broadcast (6)" << std::endl;
#ifdef _WIN32
		closesocket(discoverySocketv6);
#else
		close(discoverySocketv6);
#endif
		discoverySocketv6 = -1;
		return false;
	}	
	
	struct sockaddr_in6 anyV6;
#ifdef __APPLE_CC__
	anyV6.sin6_len = sizeof(anyV6);
#endif
	anyV6.sin6_family = AF_INET6;
	anyV6.sin6_addr = in6addr_any;
	anyV6.sin6_port = htons(portNumber);
	
	if (bind(discoverySocketv6, reinterpret_cast<const struct sockaddr *>(&anyV6), sizeof(anyV6)) < 0)
	{
		std::cout << "server: could not bind broadcast socket v6" << std::endl;
#ifdef _WIN32
		closesocket(discoverySocketv6);
#else
		close(discoverySocketv6);
#endif
		discoverySocketv6 = -1;
		return false;
	}
	
	struct ipv6_mreq mreq;
	unsigned char allnodesonlink[] = { 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
	memcpy(mreq.ipv6mr_multiaddr.s6_addr, allnodesonlink, sizeof(allnodesonlink));

	mreq.ipv6mr_interface = 0; // Not optimal, should really enumerate interfaces here and add self to all.
	if (setsockopt(discoverySocketv6, IPPROTO_IP, IPV6_JOIN_GROUP, reinterpret_cast<const char *>(&mreq), sizeof(mreq)) != 0)
	{
		std::cout << "Could not join linklocal group." << std::endl;
#ifdef _WIN32
		closesocket(discoverySocketv6);
#else
		close(discoverySocketv6);
#endif
		discoverySocketv6 = -1;
		return false;
	}
	
	if (discoverySocketv6 >= nfds) nfds = discoverySocketv6 + 1;
	
	return true;
	
}

void Server::receiveBroadcastPacket(int onSocket)
{
	unsigned long numBytesToRead;
	if (ioctl(onSocket, FIONREAD, &numBytesToRead) != 0)
		throw std::runtime_error("Could not get bytes to read.");
	
	if (numBytesToRead == 0)
	{
		// Should not happen on UDP. If it does, ignore.
		return;
	}
	
	char *data = new char[numBytesToRead];
	
	struct sockaddr_storage reallyLongAddress;
	struct sockaddr *address = reinterpret_cast<struct sockaddr *> (&reallyLongAddress);
	socklen_t addressLength = sizeof(reallyLongAddress);
	
	unsigned totalRead = recvfrom(onSocket, data, numBytesToRead, 0, address, &addressLength);
	NetworkPacket *packet = reinterpret_cast<NetworkPacket *> (data);
	
	if (totalRead < sizeof(LookingForServerPacket))
	{
		delete [] data;
		return;
	}
    packet->swapFromNetwork();
	
	// Check for packet type
	if (packet->packetType != NetworkPacket::LookingForServer)
	{
		delete [] data;
		return;
	}
	
	// Check for protocol version
	if (packet->lookingForServer.versionNumber != protocolVersionNumber)
	{
		delete [] data;
		return;
	}
	
	// Check for magic value
	if (memcmp(reinterpret_cast<const void *>(packet->lookingForServer.magicValue), serverSearchToken, 16) != 0)
	{
		delete [] data;
		return;
	}
	
	delete [] data;
	
	// Is someone we might want as client. Reply.
	NetworkPacket serverAnnounce;
	serverAnnounce.packetType = NetworkPacket::ServerAnnounce;
	serverAnnounce.packetLength = sizeof(ServerAnnouncePacket);
	serverAnnounce.serverAnnounce.versionNumber = protocolVersionNumber;
	memcpy(reinterpret_cast<void *>(serverAnnounce.serverAnnounce.magicValue), serverBroadcastToken, 16);
    unsigned packetLength = serverAnnounce.getNetworkLength();
	serverAnnounce.swapToNetwork();
    
	sendto(onSocket, reinterpret_cast<const char *>(&serverAnnounce), packetLength, 0, address, addressLength);
}

Server::Server(unsigned short aPort, Simulation *aSimulation, EnvironmentEditor *anEditor, unsigned flags)
: portNumber(aPort), simulation(aSimulation), editor(anEditor)
{
	nfds = 0;
#ifndef _WIN32	
	signal(SIGPIPE, SIG_IGN);
#endif
	bool hasIPv4Socket = startListenIPv4();
	bool hasIPv6Socket = startListenIPv6();
	if (!hasIPv4Socket && !hasIPv6Socket) throw std::runtime_error("Cannot start server!");
	
	if (flags & AllowDiscovery)
	{
		bool canBroadcastIPv4 = startBroadcastIPv4();
		bool canBroadcastIPv6 = startBroadcastIPv6();
		if (!canBroadcastIPv4 && !canBroadcastIPv6) std::cerr << "Server cannot broadcast." << std::endl;
	}
	else
	{
		discoverySocketv4 = -1;
		discoverySocketv6 = -1;
	}

		
	lastUsedClientID = 0;
	lastHue = hueDifference;
	
	if (flags & CreateLocalRobot)
	{
		localRobot = new Robot(simulation);
		registerNewRobot(localRobot);
		simulation->addRobot(localRobot);
		iteratorHasReturnedLocal = false;
		nextRobot = clients.begin();
		localSensorsChangedSinceLastUpdate = false;
		clientForLocalRobot = 0;
	}
	else
	{
		localRobot = 0;
		iteratorHasReturnedLocal = true;
		nextRobot = clients.begin();
		clientForLocalRobot = 0;
	}

}

Server::~Server()
{
	for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
	{
		closeClient(*iter);
	}
	
	if (localRobot) delete localRobot;
}

void Server::update()
{
	fd_set readsockets;
	FD_ZERO(&readsockets);
	
	// Add accept socket
	if (acceptSocketv4 != -1) FD_SET(acceptSocketv4, &readsockets);
	if (acceptSocketv6 != -1) FD_SET(acceptSocketv6, &readsockets);
	if (discoverySocketv4 != -1) FD_SET(discoverySocketv4, &readsockets);
	if (discoverySocketv6 != -1) FD_SET(discoverySocketv6, &readsockets);
	
	// Add all connected sockets
	for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
	{
		FD_SET(iter->socket, &readsockets);
	}
	
	// Check whether any sockets are ready for reading.
	struct timeval immediately = {0, 0};
	int selectResult = select(nfds, &readsockets, NULL, NULL, &immediately);
	if (selectResult < 0)
	{
		std::cout << "Error in select: ";
#ifdef _WIN32
		std::cout << WSAGetLastError();
#else
		std::cout << errno;
#endif
		std::cout << " nfds: " << nfds << std::endl;
		throw std::runtime_error("Error in select!");
	}
	
	if (selectResult > 0)
	{
		if (acceptSocketv4 != -1 && FD_ISSET(acceptSocketv4, &readsockets))
			clientConnected(acceptSocketv4);
		if (acceptSocketv6 != -1 && FD_ISSET(acceptSocketv6, &readsockets))
			clientConnected(acceptSocketv6);
		if (discoverySocketv4 != -1 && FD_ISSET(discoverySocketv4, &readsockets))
			receiveBroadcastPacket(discoverySocketv4);
		if (discoverySocketv6 != -1 && FD_ISSET(discoverySocketv6, &readsockets))
			receiveBroadcastPacket(discoverySocketv6);
		
		for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
		{
			if (!FD_ISSET(iter->socket, &readsockets)) continue;
			
			readClientData(*iter);
		}
		
		// Remove closed clients
		for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end();)
		{
			if (iter->state == ClientConnection::Closed)
			{
				clients.erase(iter);
				iter = clients.begin();
			}
			else ++iter;
		}
	}
	
	// Gather updated values for clients
    unsigned numRobots = clients.size() + 1; // +1 for local robot
	NetworkPacket *positionUpdatePacket = reinterpret_cast<NetworkPacket *> (malloc(sizeof(PositionUpdatePacket) + sizeof(PositionUpdatePacket::RobotData) * numRobots));
	positionUpdatePacket->packetType = NetworkPacket::PositionUpdate;
	positionUpdatePacket->packetLength = sizeof(PositionUpdatePacket) + sizeof(PositionUpdatePacket::RobotData) * numRobots;
	positionUpdatePacket->positionUpdate.numRobots = numRobots;
	for (unsigned i = 0; i < clients.size(); i++)
	{
		if (!clients[i].robot)
		{
			memset(&(positionUpdatePacket->positionUpdate.robot[i]), 0, sizeof(positionUpdatePacket->positionUpdate.robot[i]));
			continue;
		}
		memcpy(positionUpdatePacket->positionUpdate.robot[i].position, clients[i].robot->getPosition().c_ptr(), sizeof(float [16]));
		positionUpdatePacket->positionUpdate.robot[i].trackSpeed[0] = clients[i].robot->getLeftTrackSpeed();
		positionUpdatePacket->positionUpdate.robot[i].trackSpeed[1] = clients[i].robot->getRightTrackSpeed();
		positionUpdatePacket->positionUpdate.robot[i].isLifted = clients[i].robot->isLifted();
		positionUpdatePacket->positionUpdate.robot[i].clientID = clients[i].clientID;
		
		// Updated sensor values, only relevant to the particular client in question
		NetworkPacket sensorValuePacket;
		sensorValuePacket.packetType = NetworkPacket::SensorReadings;
		sensorValuePacket.packetLength = sizeof(SensorReadingsPacket);
		sensorValuePacket.sensorReadings.synchronizedMotors = 0;
		for (unsigned sensor = 0; sensor < 4; sensor++)
			sensorValuePacket.sensorReadings.values[sensor] = clients[i].robot->getSensorValue(sensor);
		
		for (unsigned motor = 0; motor < 3; motor++)
		{
			sensorValuePacket.sensorReadings.motorBlockCounterValues[motor] = clients[i].robot->getMotor(motor)->getBlockCounterValue();
			sensorValuePacket.sensorReadings.motorRotationCounterValues[motor] = clients[i].robot->getMotor(motor)->getRotationCounterValue();
			sensorValuePacket.sensorReadings.motorTargetCounterValues[motor] = clients[i].robot->getMotor(motor)->getTargetCounterValue();
			sensorValuePacket.sensorReadings.turnRatio[motor] = clients[i].robot->getMotor(motor)->getTurnFactor();
			
			sensorValuePacket.sensorReadings.synchronizedMotors |= (clients[i].robot->getMotorIsSynchronized(motor) << motor);
		}
		
		clients[i].sendPacket(sensorValuePacket);
	}
    // And add own robot
	if (localRobot && (clientForLocalRobot != 0))
	{
		memcpy(positionUpdatePacket->positionUpdate.robot[clients.size()].position, localRobot->getPosition().c_ptr(), sizeof(float [16]));
		positionUpdatePacket->positionUpdate.robot[clients.size()].trackSpeed[0] = localRobot->getLeftTrackSpeed();
		positionUpdatePacket->positionUpdate.robot[clients.size()].trackSpeed[1] = localRobot->getRightTrackSpeed();
		positionUpdatePacket->positionUpdate.robot[clients.size()].isLifted = localRobot->isLifted();
		positionUpdatePacket->positionUpdate.robot[clients.size()].clientID = 0;
    }
		
	sendAll(*positionUpdatePacket);
	free(positionUpdatePacket);
	
	// See whether our sensors were changed
	if (localRobot && localSensorsChangedSinceLastUpdate)
	{
		localSensorsChangedSinceLastUpdate = false;
		
		NetworkPacket robotUpdatePacket;
		robotUpdatePacket.packetType = NetworkPacket::RobotUpdate;
		robotUpdatePacket.packetLength = sizeof(RobotUpdatePacket);
		robotUpdatePacket.robotUpdate.clientID = clientForLocalRobot;
		
		for (unsigned i = 0; i < 4; i++)
		{
			robotUpdatePacket.robotUpdate.sensor[i].type = uint32_t(localRobot->getSensorType(i));
			robotUpdatePacket.robotUpdate.sensor[i].angle = localRobot->getSensorAngle(i);
			robotUpdatePacket.robotUpdate.sensor[i].pointedDown = uint8_t(localRobot->isSensorPointedDown(i));
		}
		
		sendAll(robotUpdatePacket);
	}
	
	// Actually send the data
	for (std::vector<ClientConnection>::iterator iter = clients.begin(); iter != clients.end(); ++iter)
	{
		if (::send(iter->socket, iter->unsentData, iter->unsentDataLength, 0) == -1)
		{
			std::cerr << "Could not send. Errno " << errno << std::endl;
			closeClient(*iter);
		}
		free(iter->unsentData);
		iter->unsentData = 0;
		iter->unsentDataLength = 0;
	}
}

// Robot Network interface
void Server::playTone(unsigned frequency, unsigned duration, bool loops, float gain)
{
	if (localRobot)
		localRobot->playTone(frequency, duration, loops, gain);
}

void Server::playFile(const char *name, bool loops, float gain)
{
	if (localRobot)
		localRobot->playFile(name, loops, gain);
}

void Server::commitMotorChanges()
{
	// Ignore
}

void Server::updatedCellState(unsigned x, unsigned z)
{
	bool isWall = editor->getCellIsWall(x, z);
	float color = editor->getCellShade(x, z);
	
	NetworkPacket cellUpdatePacket;
	cellUpdatePacket.packetType = NetworkPacket::SetCell;
	cellUpdatePacket.packetLength = sizeof(SetCellPacket);
	cellUpdatePacket.setCell.x = x;
	cellUpdatePacket.setCell.z = z;
	cellUpdatePacket.setCell.cell = (isWall ? 0x80 : 0x00) + uint8_t(color * 127.0f);
	
	sendAll(cellUpdatePacket);	
}

void Server::setSensorAngle(unsigned sensor, float angleInDegrees) throw(std::out_of_range)
{
	if (!localRobot) return;
	
	localRobot->setSensorAngle(sensor, angleInDegrees);
	localSensorsChangedSinceLastUpdate = true;
}

void Server::setIsSensorPointedDown(unsigned sensor, float isit) throw(std::out_of_range)
{
	if (!localRobot) return;
	
	localRobot->setIsSensorPointedDown(sensor, isit);
	localSensorsChangedSinceLastUpdate = true;
}

void Server::setSensorType(unsigned sensor, Robot::SensorType type) throw(std::invalid_argument)
{
	if (!localRobot) return;
	
	localRobot->setSensorType(sensor, type);
	localSensorsChangedSinceLastUpdate = true;
}

void Server::setIsLifted(bool isRaised) throw()
{
	if (!localRobot) return;
	
	localRobot->setIsLifted(isRaised);
}

void Server::moveLifted(const union float4 &diff) throw()
{
	if (!localRobot) return;
	
	localRobot->moveDirectly(diff);
}

void Server::rotateLifted(float radians) throw()
{
	if (!localRobot) return;
	
	localRobot->rotate(radians);
}

void Server::setRobotTurnSpeed(float speed) throw()
{
	if (!localRobot) return;
	
	localRobot->setLiftedTurnSpeed(speed);
}


Robot *Server::getLocalModifiableRobot() throw()
{
	return localRobot;
}
const Robot *Server::getLocalRobot() const throw()
{
	return localRobot;
}

Robot *Server::getNextRobot() throw()
{
	if (nextRobot == clients.end())
	{
		if (!iteratorHasReturnedLocal && localRobot && (clientForLocalRobot != 0))
		{
			iteratorHasReturnedLocal = true;
			return localRobot;
		}
		else
		{
			nextRobot = clients.begin();
			iteratorHasReturnedLocal = false;
			return 0;
		}
	}
	else
	{
		Robot *result = nextRobot->robot;
		++nextRobot;
		return result;
	}
}


