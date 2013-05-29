/*
 *  ServerBrowser.cpp
 *  mindstormssimulation
 *
 *  Created by Torsten Kammer on 25.06.10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "ServerBrowser.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#define ioctl ioctlsocket
#else
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include <stdexcept>

#include "NetworkConstants.h"
#include "NetworkPacket.h"

namespace
{
	const unsigned updatesBetweenBroadcastsActive = 4;
	const unsigned updatesBetweenBroadcastsLessActive = 40;
}

bool ServerBrowser::readPacketFrom(int onSocket)
{
	unsigned long numBytesToRead;
	if (ioctl(onSocket, FIONREAD, &numBytesToRead) != 0)
		throw std::runtime_error("Could not get bytes to read.");
	
	if (numBytesToRead == 0)
	{
		// Should not happen on UDP. If it does, ignore.
		return false;
	}
	
	char *data = new char [numBytesToRead];
	
	struct sockaddr_storage address;
	socklen_t addressLength = sizeof(address);
	
	unsigned totalRead = recvfrom(onSocket, data, numBytesToRead, 0, reinterpret_cast<struct sockaddr *> (&address), &addressLength);
	NetworkPacket *packet = reinterpret_cast<NetworkPacket *> (data);
	
	if (totalRead < sizeof(ServerAnnouncePacket))
	{
		delete [] data;
		return false;
	}
    packet->swapFromNetwork();
	
	// Check for packet type
	if (packet->packetType != NetworkPacket::ServerAnnounce)
	{
		delete [] data;
		return false;
	}
	
	// Check for protocol version
	if (packet->serverAnnounce.versionNumber != protocolVersionNumber)
	{
		delete [] data;
		return false;
	}
	
	// Check for magic value
	if (memcmp(reinterpret_cast<const void *>(packet->serverAnnounce.magicValue), serverBroadcastToken, 16) != 0)
	{
		delete [] data;
		return false;
	}
	
	delete [] data;
	
	if (!lastServerAddress) lastServerAddress = new sockaddr_storage;
	
	memcpy(lastServerAddress, &address, sizeof(sockaddr_storage));
	return true;
}

bool ServerBrowser::startBroadcastIPv4()
{
	discoverySocketv4 = socket(PF_INET, SOCK_DGRAM, 0);
	if (discoverySocketv4 == -1) return false;
	
	int socketOptionValue = 1;
	if (setsockopt(discoverySocketv4, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char *>(&socketOptionValue), sizeof(socketOptionValue)) == -1)
	{
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
	anyV4.sin_port = /* htons(portNumber) */ htons(0);
	
	if (bind(discoverySocketv4, reinterpret_cast<const struct sockaddr *>(&anyV4), sizeof(anyV4)) < 0)
	{
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

bool ServerBrowser::startBroadcastIPv6()
{
	discoverySocketv6 = socket(PF_INET6, SOCK_DGRAM, 0);
	if (discoverySocketv6 == -1) return false;
	
	int socketOptionValue = 1;
	if (setsockopt(discoverySocketv6, SOL_SOCKET, SO_BROADCAST, reinterpret_cast<const char *> (&socketOptionValue), sizeof(socketOptionValue)) == -1)
	{
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


ServerBrowser::ServerBrowser(unsigned short port)
: portNumber(port)
{
	discoverySocketv4 = -1;
	discoverySocketv6 = -1;
	
	nfds = 0;
	
	bool canBroadcastIPv4 = startBroadcastIPv4();
	bool canBroadcastIPv6 = startBroadcastIPv6();
	if (!canBroadcastIPv4 && !canBroadcastIPv6) throw std::runtime_error("Can’t start server browser");
	
	lastServerAddress = 0;
	lastServerIsIPv6 = false;
	updatesSinceLastBroadcast = 0;
}

ServerBrowser::~ServerBrowser()
{
#ifdef _WIN32
	if (discoverySocketv4 != -1) closesocket(discoverySocketv4);
	if (discoverySocketv6 != -1) closesocket(discoverySocketv6);
#else
	if (discoverySocketv4 != -1) close(discoverySocketv4);
	if (discoverySocketv6 != -1) close(discoverySocketv6);
#endif
	
	if (lastServerAddress) delete lastServerAddress;
	
	discoverySocketv4 = -1;
	discoverySocketv6 = -1;
	lastServerAddress = 0;
}

void ServerBrowser::update() throw(std::runtime_error)
{
	while(true)
	{
		fd_set readsockets;
		FD_ZERO(&readsockets);
		
		if (discoverySocketv4 != -1) FD_SET(discoverySocketv4, &readsockets);
		if (discoverySocketv6 != -1) FD_SET(discoverySocketv6, &readsockets);
		
		struct timeval immediately = {0, 0};
		int selectResult = select(nfds, &readsockets, NULL, NULL, &immediately);
		if (selectResult == 0) break;
		
		if (discoverySocketv4 != -1 && FD_ISSET(discoverySocketv4, &readsockets))
		{
			if (readPacketFrom(discoverySocketv4))
				lastServerIsIPv6 = false;
		}
		else if (discoverySocketv6 != -1 && FD_ISSET(discoverySocketv6, &readsockets))
			if (readPacketFrom(discoverySocketv6))
				lastServerIsIPv6 = true;
	}
	
	if (updatesSinceLastBroadcast == 0)
	{
		NetworkPacket searchingPacket;
		searchingPacket.packetType = NetworkPacket::LookingForServer;
		searchingPacket.packetLength = sizeof(LookingForServerPacket);
		searchingPacket.lookingForServer.versionNumber = protocolVersionNumber;
		memcpy(searchingPacket.lookingForServer.magicValue, serverSearchToken, sizeof(searchingPacket.lookingForServer.magicValue));
        unsigned packetLength = searchingPacket.getNetworkLength();
        searchingPacket.swapToNetwork();
		
		if (discoverySocketv4 != -1)
		{	
			struct sockaddr_in ipv4Broadcast;
#ifdef __APPLE_CC__
			ipv4Broadcast.sin_len = sizeof(ipv4Broadcast);
#endif
			ipv4Broadcast.sin_family = AF_INET;
			ipv4Broadcast.sin_addr.s_addr = htonl(INADDR_BROADCAST); // Is that correct?
			ipv4Broadcast.sin_port = htons(portNumber);
			
			int result = sendto(discoverySocketv4, reinterpret_cast<const char *>(&searchingPacket), packetLength, 0, reinterpret_cast<const struct sockaddr *> (&ipv4Broadcast), sizeof(ipv4Broadcast));
			if (result == -1) throw std::runtime_error("send ipv4 failed");
		}
		if (discoverySocketv6 != -1)
		{
			struct sockaddr_in6 ipv6Broadcast;
#ifdef __APPLE_CC__
			ipv6Broadcast.sin6_len = sizeof(ipv6Broadcast);
#endif
			ipv6Broadcast.sin6_family = AF_INET6;
			unsigned char allnodesonlink[] = { 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 };
			memcpy(ipv6Broadcast.sin6_addr.s6_addr, allnodesonlink, sizeof(allnodesonlink));
			
			ipv6Broadcast.sin6_port = htons(portNumber);
			
			int result = sendto(discoverySocketv6, reinterpret_cast<const char *>(&searchingPacket), packetLength, 0, reinterpret_cast<const struct sockaddr *> (&ipv6Broadcast), sizeof(ipv6Broadcast));
			if (result == -1) throw std::runtime_error("send ipv6 failed");
		}
	}
	if (!lastServerAddress)
		updatesSinceLastBroadcast = (updatesSinceLastBroadcast + 1) % updatesBetweenBroadcastsActive;
	else
		updatesSinceLastBroadcast = (updatesSinceLastBroadcast + 1) % updatesBetweenBroadcastsLessActive;
}
